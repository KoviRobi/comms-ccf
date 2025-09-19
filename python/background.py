from asyncio import EventLoop, Task


class BackgroundTasks:
    def __init__(self, loop: EventLoop) -> None:
        self._tasks: set[Task] = set()
        self._loop = loop

    def add(self, async_fn, *args, **kwargs):
        channel_task = self._loop.create_task(async_fn(*args, **kwargs))
        self._tasks.add(channel_task)
        channel_task.add_done_callback(self._tasks.discard)
