#!/bin/bash
# PORTMASTER: GodotTest.zip, GodotTest.sh

XDG_DATA_HOME=${XDG_DATA_HOME:-$HOME/.local/share}

if [ -d "/opt/system/Tools/PortMaster/" ]; then
  controlfolder="/opt/system/Tools/PortMaster"
elif [ -d "/opt/tools/PortMaster/" ]; then
  controlfolder="/opt/tools/PortMaster"
elif [ -d "$XDG_DATA_HOME/PortMaster/" ]; then
  controlfolder="$XDG_DATA_HOME/PortMaster"
else
  controlfolder="/roms/ports/PortMaster"
fi

source $controlfolder/control.txt
if [ -z ${TASKSET+x} ]; then
  source $controlfolder/tasksetter
fi

[ -f "${controlfolder}/mod_${CFW_NAME}.txt" ] && source "${controlfolder}/mod_${CFW_NAME}.txt"
get_controls

export gamedir="/$directory/ports/Dev/SDL2Test"

echo $gamedir
cd $gamedir

export LIBGL_ES=2
export LIBGL_GL=21
export LIBGL_FB=4

$TASKSET ./SDL2Test 

$ESUDO umount "${weston_dir}"

pm_finish
