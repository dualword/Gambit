#include "NamedLock.h"
#include <cstdio>

int main()
{
    // On Unix, the lock is a locked file.
    // On Windows, the lock is a mutex.
    //
    // On Windows, the mutex can be either:
    //     - Local (creates mutex in the session namespace; i.e., for
    //       the user creating the mutex).
    //     - Global (creates mutex in the global namespace).
    //
    //     It is good practice to add a unique identifier to the mutex
    //     name, so collisions with other programs are less likely.
    //
    // On Unix, the locality of the lock can simply be controlled by
    // its filename.
    // For example, if one uses "/tmp/my_global_lock" as the lock
    // filename, then every instance using that filename will see that
    // lock file.
    // On the other hand, if one uses
    // "$HOME/.config/my_app/my_local_lock", then the lock is
    // effectively bound to the user creating the lock file.
    NamedLock namedLock(
        // Unix parameter. Ignored on Windows.
        "my_lock_file",
        // Windows parameters. Ignored on Unix.
        NamedLock::Local,
        "my_lock-uniqueIDgoeshere");

    if (!namedLock.try_lock())
        printf("did _NOT_ obtain lock\n");
    else
        printf("obtained lock\n");

    printf("Press ENTER to exit ...\n");
    char buf[2];
    fgets(buf, sizeof(buf), stdin);

    return 0;
}
