from __future__ import annotations
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
        ctx.run('cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..')


@task(default=True)
def make(ctx):
    with ctx.cd(str(BUILD_PATH)):
        ctx.run('make')
