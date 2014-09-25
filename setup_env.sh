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

$(echo $0 | grep -q "setup_env\.sh")
if [ $? -eq 0 ]; then
    echo Please source this script \(. setup_env.sh\).
    exit
fi

alias c="cmake -G \"Unix Makefiles\" -D \"CMAKE_BUILD_TYPE=Debug\" ."
alias cr="cmake -G \"Unix Makefiles\" -D \"CMAKE_BUILD_TYPE=Release\" ."
alias b="make"
alias r="./gambitchess"
alias br="b && r"
alias cl="make clean"
