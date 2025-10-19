"""
Keep track of background tasks so their exceptions can be read.
"""

from asyncio import AbstractEventLoop, Task


class BackgroundTasks:
    def __init__(self, loop: AbstractEventLoop) -> None:
        self._tasks: set[Task] = set()
        self._loop = loop
        self.suppress_exceptions = False

    def add(self, async_fn, *args, **kwargs):
        channel_task = self._loop.create_task(async_fn(*args, **kwargs))
        self._tasks.add(channel_task)
        channel_task.add_done_callback(self._discard)

    def _discard(self, task: Task):
        self._tasks.discard(task)
        exc = None
        try:
            exc = task.exception()
        except BaseException as e:
            exc = e
        finally:
            if exc and not self.suppress_exceptions:
                raise exc
