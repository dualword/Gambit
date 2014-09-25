NamedLock
=========

The NamedLock class is cross-platform.

See "example.cpp" for an example of how one can use the NamedLock.

It can be used to implement an instance lock, to check whether another
instance of one's program is running.

On Unix, NamedLock uses a lock file, which it creates and locks to
acquire the lock.
We check whether we can lock the file to check whether another instance
is holding the lock, and not merely check the existence of the lock
file. This is done to guard against system crashes and power losses, as
both could result in the file still existing. However, lock states are
lost when a system crash or power loss occurs.
The lock file is removed when the holder of the lock releases it.

On Windows, NamedLock uses a mutex, which it creates to acquire the
lock.
