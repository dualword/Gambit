#!/bin/sh
#
# Written by Jelle Geerts (jellegeerts@gmail.com).
#
# To the extent possible under law, the author(s) have dedicated all
# copyright and related and neighboring rights to this software to
# the public domain worldwide. This software is distributed without
# any warranty.
#
# You should have received a copy of the CC0 Public Domain Dedication
# along with this software.
# If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.

REPOSITORY_URL=http://svn.code.sourceforge.net/p/gambitchess/code/trunk

cd "`dirname "$0"`"

# Open the script in a terminal, the user shouldn't have to open a terminal to execute this script
# in.
if [ "$1" != '--terminal-is-opened' ]; then
    title='Gambit: automated build'
    args=--terminal-is-opened
    if which konsole >/dev/null 2>&1; then
        exec konsole --title "$title" -e "$0" "$args"
    elif which xfce4-terminal >/dev/null 2>&1; then
        exec xfce4-terminal --title "$title" --execute "$0" "$args"
    elif which Terminal >/dev/null 2>&1; then
        exec Terminal --title "$title" --execute "$0" "$args"
    elif which gnome-terminal >/dev/null 2>&1; then
        exec gnome-terminal --title "$title" --execute "$0" "$args"
    else
        exec xterm -title "$title" -e "$0" "$args"
    fi
    exit 1
fi

pause_before_exit() {
    echo 'Press RETURN to exit this script ...'
    read ignore
}

abort() {
    echo 'Something went wrong.' >&2
    pause_before_exit
    exit 1
}

asroot() {
    echo "About to execute the following command with super-user privileges:"
    echo "  $@"
    echo
    if which sudo >/dev/null 2>&1; then
        echo "If necessary, please authenticate to \`sudo' to continue."
        echo "(Usually, you should enter your own user password.)"
        sudo "$@"
    elif which su >/dev/null 2>&1; then
        echo "Please enter the root password to continue."
        # We pass the username 'root' to 'su', as this is necessary for at
        # least FreeBSD.
        eval su root -c \""$@"\"
    else
        echo "No method for privilege elevation available." >&2
        abort
    fi
}

install_required_packages_if_necessary() {
    yum --help >/dev/null 2>&1
    local no_yum=$?

    apt-get --help >/dev/null 2>&1
    local no_aptget=$?

    [ $(uname -s) != 'FreeBSD' ] >/dev/null 2>&1
    local is_freebsd=$?

    pkg help >/dev/null 2>&1
    local no_pkg=$?

    # For recognized platforms, use the officially supported package manager.
    if [ -e /etc/debian_version ]; then
        if [ $no_aptget -ne 0 ]; then
            echo "The \`apt-get' package manager utility does not seem to be installed." >&2
            return 1
        fi
    elif [ -e /etc/fedora-release ]; then
        if [ $no_yum -ne 0 ]; then
            echo "The \`yum' package manager utility does not seem to be installed." >&2
            return 1
        fi
    elif [ $is_freebsd -ne 0 ]; then
        if [ $no_pkg -ne 0 ]; then
            echo "The \`pkg' package manager utility does not seem to be installed." >&2
            return 1
        fi
    else
        # For unrecognized platforms, use the first recognized package manager that is available.
        if [ $no_aptget -ne 0 ] && [ $no_yum -ne 0 ] && [ $no_pkg -ne 0 ]; then
            echo "No package manager utility available (tried \`apt-get', \`yum', and \`pkg')." >&2
            return 1
        fi
    fi

    local package_names_deb="libqt4-dev cmake make gcc g++ subversion"
    local package_names_rpm="qt qt-devel cmake make gcc gcc-c++ subversion"
    local package_names_freebsd="qt4 cmake gmake gcc subversion"

    are_required_deb_packages_consistent() {
        local statuses
        statuses="`dpkg-query --show --showformat='${Status}\n' $package_names_deb 2>&1`"
        if [ $? -eq 0 ]; then
            local packages_not_ok=0
            local IFS='
'
            for status in $statuses; do
                # It's legitimate for a package status to be something other than the usual
                # "install ok installed", such as "hold ok installed". So, only check for the
                # latter, and ignore the first field.
                if [ `echo $status | cut -d ' ' -f 2-` != 'ok installed' ]; then
                    packages_not_ok=1
                fi
            done
            return $packages_not_ok
        fi
        return 1
    }

    local msg='Installing required packages ...'
    if [ $no_aptget -eq 0 ]; then
        if are_required_deb_packages_consistent; then
            # Packages already installed and in a consistent state.
            return 0
        fi

        echo 'Updating package index files, please wait ...'
        asroot apt-get update

        echo "$msg"
        asroot apt-get install $package_names_deb

        # It may be that some packages were already installed but not in a consistent state. Simply
        # running 'apt-get install $some_packages' will not fix that. But running
        # 'apt-get -f install' will, so let's do that if necessary.
        if ! are_required_deb_packages_consistent; then
            asroot apt-get -f install
        fi
    elif [ $no_yum -eq 0 ]; then
        rpm --query $package_names_rpm >/dev/null 2>&1
        if [ $? -eq 0 ]; then
            # Packages already installed and in a consistent state.
            return 0
        fi

        echo "$msg"
        asroot yum install $package_names_rpm
    elif [ $is_freebsd -ne 0 ] && [ $no_pkg -eq 0 ]; then
        pkg info $package_names_freebsd >/dev/null 2>&1
        if [ $? -eq 0 ]; then
            # Packages already installed and in a consistent state.
            return 0
        fi

        echo "$msg"
        asroot pkg install $package_names_freebsd
    else
        # NOTREACHED
        echo "NOTREACHED reached." >&2
        abort
    fi
}

# Shows a textbox such as:
# ###########
# # Example #
# ###########
#
# This command accepts a variable number of parameters.
#
# NOTE:
# Horizontal TAB characters in any of the parameters are treated as if they
# take up one character of space when displayed. Thus, the frame around the
# textbox may appear distorted in such cases.
textbox() {
    local boxwidth=79
    local framechar='#'
    local IFS='
'

    local i=$boxwidth
    local frame=
    while [ $i -ne 0 ]; do
        i=$(($i - 1))
        frame="$frame$framechar"
    done
    echo "$frame"

    echo "$@" | while read line; do
        local padding=''
        local i=$(($boxwidth - ${#line} - 4))
        while [ $i -gt 0 ]; do
            i=$(($i - 1))
            padding="$padding "
        done
        printf '# %s%s #\n' "$line" "$padding"
    done

    echo "$frame"
}

install_required_packages_if_necessary || abort

gambitdir=Gambit
gambitbinary=gambitchess

# Check out the code if necessary.
code_may_be_outdated=1
if ! [ -d "$gambitdir" ]; then
    echo 'Checking out the code from the repository, please wait ...'
    svn co "$REPOSITORY_URL" "$gambitdir"
    if [ $? -ne 0 ]; then
        echo 'Failed checking out the code. Please try again later.' >&2
        abort
    fi
    code_may_be_outdated=0
fi

cd "$gambitdir" || abort

# We may not have checked out the code, in which case the code that was already there might be
# outdated, so update it.
if [ $code_may_be_outdated -eq 1 ]; then
    echo 'Updating the code, please wait ...'
    svn up
    if [ $? -ne 0 ]; then
        echo 'Failed updating the code. Please try again later.' >&2
        abort
    fi
fi

# Build the chess engine Gupta.
cd engine/gupta || abort
gmake release || abort
cd "$OLDPWD" || abort

# Build Gambit.
sh clean_all.sh
. ./setup_env.sh
c
b
if [ $? -ne 0 ]; then
    echo 'Could not build Gambit.' >&2
    echo 'Perhaps the current Gambit code is not in a compilable state, in which case you' >&2
    echo 'may try again later.' >&2
    abort
fi

textbox "The build seems to have succeeded.

To run Gambit now or in the future, do the following:
  1) Open the '$gambitdir' folder.
  2) Open the '$gambitbinary' executable."
echo
pause_before_exit
