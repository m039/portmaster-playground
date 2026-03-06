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

export gamedir="/$directory/ports/Godot3Test"

echo $gamedir
cd $gamedir

$GPTOKEYB "frt_3.6" &

./frt_3.6 --main-pack "Godot 3 Test.pck"

$ESUDO kill -9 $(pidof gptokeyb)

pm_finish