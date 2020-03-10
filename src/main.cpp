/**
 *   #, #,         CCCCCC  VV    VV MM      MM RRRRRRR
 *  %  %(  #%%#   CC    CC VV    VV MMM    MMM RR    RR
 *  %    %## #    CC        V    V  MM M  M MM RR    RR
 *   ,%      %    CC        VV  VV  MM  MM  MM RRRRRR
 *   (%      %,   CC    CC   VVVV   MM      MM RR   RR
 *     #%    %*    CCCCCC     VV    MM      MM RR    RR
 *    .%    %/
 *       (%.      Computer Vision & Mixed Reality Group
 *                For more information see <http://cvmr.info>
 *
 * This file is part of RBOT.
 *
 *  @copyright:   RheinMain University of Applied Sciences
 *                Wiesbaden Rüsselsheim
 *                Germany
 *     @author:   Henning Tjaden
 *                <henning dot tjaden at gmail dot com>
 *    @version:   1.0
 *       @date:   30.08.2018
 *
 * RBOT is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * RBOT is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with RBOT. If not, see <http://www.gnu.org/licenses/>.
 */

#include <iostream>
#include <string>

#include <QApplication>
#include <QThread>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>

#include "Arguments.hpp"
#include "Pose.hpp"
#include "object3d.h"
#include "pose_estimator6d.h"

using namespace std;
using namespace cv;
using TrackbarAction = std::function<void(int)>;

cv::Mat drawResultOverlay(const vector<Object3D*>& objects,
                          const cv::Mat& frame)
{
    // render the models with phong shading
    RenderingEngine::Instance()->setLevel(0);

    vector<Point3f> colors;
    colors.push_back(Point3f(1.0, 0.5, 0.0));
    // colors.push_back(Point3f(0.2, 0.3, 1.0));
    RenderingEngine::Instance()->renderShaded(
        vector<Model*>(objects.begin(), objects.end()), GL_FILL, colors, true);

    // download the rendering to the CPU
    Mat rendering =
        RenderingEngine::Instance()->downloadFrame(RenderingEngine::RGB);

    // download the depth buffer to the CPU
    Mat depth =
        RenderingEngine::Instance()->downloadFrame(RenderingEngine::DEPTH);

    // compose the rendering with the current camera image for demo purposes
    // (can be done more efficiently directly in OpenGL)
    Mat result = frame.clone();
    for (int y = 0; y < frame.rows; y++)
    {
        for (int x = 0; x < frame.cols; x++)
        {
            Vec3b color = rendering.at<Vec3b>(y, x);
            if (depth.at<float>(y, x) != 0.0f)
            {
                constexpr auto a = 0.4;
                auto r = result.at<Vec3b>(y, x)[0];
                auto g = result.at<Vec3b>(y, x)[1];
                auto b = result.at<Vec3b>(y, x)[2];
                result.at<Vec3b>(y, x)[0] = color[2] * a + r * (1 - a);
                result.at<Vec3b>(y, x)[1] = color[1] * a + g * (1 - a);
                result.at<Vec3b>(y, x)[2] = color[0] * a + b * (1 - a);
            }
        }
    }
    return result;
}

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    auto const args = fds::Arguments{argc, argv};

    // Camera image size
    auto const width = 640;
    auto const height = 480;

    auto video = cv::VideoCapture{};

    if (!video.open(args.get_device_path(), cv::CAP_V4L2))
    {
        std::cerr << "Unable to open video device\n";
        return -1;
    }

    if (!video.set(cv::CAP_PROP_FRAME_WIDTH, width))
    {
        std::cerr << "Unable to set capture frame width\n";
        return -1;
    }

    if (!video.set(cv::CAP_PROP_FRAME_HEIGHT, height))
    {
        std::cerr << "Unable to set capture frame height\n";
        return -1;
    }

    // Near and far plane of the OpenGL view frustum.
    auto const zNear = 10.0f;
    auto const zFar = 10000.0f;

    // Camera instrinsics
    // FDS hardcoded from tracker/local/tracker.yaml
    Matx33f K = Matx33f(627.746, 0, 327.113, 0, 627.746, 242.4199, 0, 0, 1);
    Matx14f distCoeffs = Matx14f(0.0, -0.0678, 0.0153, 0.0);

    // distances for the pose detection template generation
    /* vector<float> distances = {400.0f, 800.0f, 1200.0f}; */
    // FDS
    vector<float> distances = {800.0f, 1000.0f, 1200.0f};

    auto pose = fds::Pose{15, -45, args.get_z_distance(), 115, 0, 45};

    auto object = Object3D(args.get_object_path(),
                           pose.tx,
                           pose.ty,
                           pose.tz,
                           pose.alpha,
                           pose.beta,
                           pose.gamma,
                           1.0,
                           0.55f,
                           distances);

    auto objects = std::vector<Object3D*>{&object};

    auto poseEstimator = PoseEstimator6D{width,
                                         height,
                                         zNear,
                                         zFar,
                                         K,
                                         distCoeffs,
                                         objects,
                                         args.get_generate_object_templates()};

    // move the OpenGL context for offscreen rendering to the current thread, if
    // run in a seperate QT worker thread (unnessary in this example)
    // RenderingEngine::Instance()->getContext()->moveToThread(this);

    // active the OpenGL context for the offscreen rendering engine during pose
    // estimation
    RenderingEngine::Instance()->makeCurrent();

    bool showHelp = true;

    Mat frame;

    constexpr auto window_name = "RBOT";

    cv::namedWindow(window_name);

    cv::TrackbarCallback trackbarCallback = [](int pos, void* userdata) {
        (*(TrackbarAction*)userdata)(pos);
    };

    int tx = pose.tx;
    int tx_max = 100;

    TrackbarAction handleX = [&object, &pose](int newTx) {
        pose.setTx(newTx);
        object.setPose(pose);
    };

    cv::createTrackbar(
        "x", window_name, &tx, tx_max, trackbarCallback, (void*)&handleX);

    int ty = pose.ty;
    int ty_max = 100;

    TrackbarAction handleY = [&object, &pose](int newTy) {
        pose.setTy(newTy);
        object.setPose(pose);
    };

    cv::createTrackbar(
        "y", window_name, &ty, ty_max, trackbarCallback, (void*)&handleY);

    int tz = pose.tz;
    int tz_max = 1500;

    TrackbarAction handleZ = [&object, &pose](int newTz) {
        pose.setTz(newTz);
        object.setPose(pose);
    };

    cv::createTrackbar(
        "z", window_name, &tz, tz_max, trackbarCallback, (void*)&handleZ);

    int alpha = pose.alpha;
    auto alpha_max = 360;

    TrackbarAction handleAlpha = [&object, &pose](int newAlpha) {
        pose.setAlpha(newAlpha);
        object.setPose(pose);
    };

    cv::createTrackbar("alpha",
                       window_name,
                       &alpha,
                       alpha_max,
                       trackbarCallback,
                       (void*)&handleAlpha);

    int beta = pose.beta;
    auto beta_max = 360;

    TrackbarAction handleBeta = [&object, &pose](int newBeta) {
        pose.setBeta(newBeta);
        object.setPose(pose);
    };

    cv::createTrackbar("beta",
                       window_name,
                       &beta,
                       beta_max,
                       trackbarCallback,
                       (void*)&handleBeta);

    int gamma = pose.gamma;
    auto gamma_max = 360;

    TrackbarAction handleGamma = [&object, &pose](int newGamma) {
        pose.setGamma(newGamma);
        object.setPose(pose);
    };

    cv::createTrackbar("gamma",
                       window_name,
                       &gamma,
                       gamma_max,
                       trackbarCallback,
                       (void*)&handleGamma);
    
    while (true)
    {
        // Read the next frame from the webcam.
        video >> frame;

        if (frame.empty())
        {
            continue;
        }

        // the main pose uodate call
        poseEstimator.estimatePoses(frame, true, false);

        // render the models with the resulting pose estimates ontop of the
        // input image
        Mat result = drawResultOverlay(objects, frame);

        if (showHelp)
        {
            putText(result,
                    "Press '1' to initialize",
                    Point(150, 250),
                    FONT_HERSHEY_DUPLEX,
                    1.0,
                    Scalar(255, 255, 255),
                    1);
            putText(result,
                    "or 'c' to quit",
                    Point(205, 285),
                    FONT_HERSHEY_DUPLEX,
                    1.0,
                    Scalar(255, 255, 255),
                    1);
        }

        imshow(window_name, result);

        int key = cv::waitKey(1 /* delay */);

        // start/stop tracking the first object
        if (key == (int)'1')
        {
            poseEstimator.toggleTracking(frame, 0, false);
            poseEstimator.estimatePoses(frame, true, false);
            showHelp = !showHelp;
        }
        if (key == (int)'2') // the same for a second object
        {
            // poseEstimator->toggleTracking(frame, 1, false);
            // poseEstimator->estimatePoses(frame, false, false);
        }
        // reset the system to the initial state
        if (key == (int)'r')
            poseEstimator.reset();
        // stop the demo
        if (key == (int)'c')
            break;
    }

    // deactivate the offscreen rendering OpenGL context
    RenderingEngine::Instance()->doneCurrent();

    // clean up
    RenderingEngine::Instance()->destroy();
    video.release();
}
