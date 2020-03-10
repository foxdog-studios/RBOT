from __future__ import annotations
import os
import shlex
from pathlib import Path

from invoke import task

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
        'cmake',
        'make',
        'opencv',
    ])
    cmake(ctx)


@task
def cmake(ctx):
    BUILD_PATH.mkdir(parents=True, exist_ok=True)
    with ctx.cd(str(BUILD_PATH)):
        command = shlex.join([
            'cmake',
            '-DCMAKE_EXPORT_COMPILE_COMMANDS=ON',
            '..'
        ])

        print(command)

        ctx.run(command, env={
            'CC': 'clang',
            'CCFLAGS': '-Wall -O3 -flto',
            'CXX': 'clang++',
            'CXXFLAGS': '-Wall -O3 -flto',
        })


@task(default=True)
def make(ctx):
    with ctx.cd(str(BUILD_PATH)):
        args = ['make']
        cpu_count = os.cpu_count()
        if cpu_count is not None:
            args.append(f'-j{cpu_count}')
        command = shlex.join(args)
        print(command)
        ctx.run(command, pty=True)


@task
def run(ctx):
    with ctx.cd(str(REPO_PATH)):
        ctx.run(f'{BUILD_PATH}/bin/RBOT /dev/c920-* data/ship-hull.obj 1000')
