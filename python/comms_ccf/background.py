"""
Keep track of background tasks so their exceptions can be read.
"""

import traceback
from asyncio import AbstractEventLoop, Task, as_completed


class BackgroundTasks:
    def __init__(self, loop: AbstractEventLoop) -> None:
        self._tasks: set[Task] = set()
        self._loop = loop
        self.suppress_exceptions: set[type] = set()

    def add(self, async_fn, *args, **kwargs) -> Task:
        task = self._loop.create_task(async_fn(*args, **kwargs))
        self._tasks.add(task)
        task.add_done_callback(self._discard)
        return task

    def cancel(self):
        for task in self._tasks:
            task.cancel()

    async def wait(self, timeout: float | None = None):
        err = False
        for res in as_completed(self._tasks, timeout=timeout):
            try:
                await res
            except BaseException as exc:
                # Handled by _discard, just need to know if we should err
                if not any(isinstance(exc, ty) for ty in self.suppress_exceptions):
                    err = True
        if err:
            raise SystemExit(1)

    def _discard(self, task: Task):
        self._tasks.discard(task)
        exc = None
        try:
            exc = task.exception()
        except BaseException as e:
            exc = e
        finally:
            if exc and not any(isinstance(exc, ty) for ty in self.suppress_exceptions):
                traceback.print_exception(exc)
