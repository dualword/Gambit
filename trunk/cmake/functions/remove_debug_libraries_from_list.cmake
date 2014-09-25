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

function(remove_debug_libraries_from_list list)
    list(LENGTH ${list} LEN)
    math(EXPR RANGE "${LEN} - 1")
    foreach(INDEX RANGE ${RANGE})
      list(GET ${list} ${INDEX} ITEM)
      string(COMPARE EQUAL ${ITEM} "debug" IS_EQUAL)
      if(ITEM STREQUAL "debug")
        math(EXPR DEL_INDEX "${INDEX} + 1")
        set(INDICES ${INDICES} ${INDEX} ${DEL_INDEX})
      endif(ITEM STREQUAL "debug")
    endforeach(INDEX)

    if(INDICES)
        list(REMOVE_AT ${list} ${INDICES})
        set(${list} ${${list}} PARENT_SCOPE)
    endif(INDICES)
endfunction(remove_debug_libraries_from_list)
