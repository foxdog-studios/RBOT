from __future__ import annotations
import shlex
from pathlib import Path

from invoke import Context, task

import charm


REPO_PATH = Path(__file__).resolve(strict=True).parent
BUILD_PATH = REPO_PATH / 'build'
project = charm.Project(REPO_PATH)


@task(name='clean')
def clean_task(ctx, dry_run=False):
    project.clean(ctx, dry_run=dry_run)


@task
def setup(ctx, clean=True):
    if clean:
        clean_task(ctx)
    charm.setup(ctx, packages=[
        'assimp',
        'cxxopts',
        'meson',
        'ninja',
        'opencv',
    ])


@task
def meson(ctx):
    with project.cd(ctx):
        ctx.run(f'meson {BUILD_PATH}', pty=True)


def run_ninja(ctx: Context, *targets: str, hide: bool = False) -> None:
    command = shlex.join(['ninja', '-C', str(BUILD_PATH), *targets])
    charm.print_shell_command(command)
    ctx.run(command, hide=hide, pty=True)


@task(name='ninja', default=True)
def ninja_task(ctx):
    run_ninja(ctx)


@task
def rbot(ctx, object=None, shm=False, ninja=True):
    if object is None:
        object = str(project / 'data/ship-hull.obj')
    if ninja:
        run_ninja(ctx, 'rbot')
    args = [str(BUILD_PATH / 'rbot')]
    if shm:
        args.append('-v')
        args.append('shm')
    args.append(object)
    command = shlex.join(args)
    env = {'RBOT_SHADERS_PATH': str(project / 'src')}
    charm.print_shell_command(command, env=env)
    ctx.run(command, env=env)


@task
def shmvideo(ctx, ninja=True):
    if ninja:
        run_ninja(ctx, 'shmvideo')
    command = shlex.join([str(BUILD_PATH / 'shmvideo')])
    charm.print_shell_command(command)
    ctx.run(command)
