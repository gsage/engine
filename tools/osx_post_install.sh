# replace symlinks with originals for app bundle

#!/usr/bin/bash

app="$1"
folders=("Contents/Frameworks" "Contents/Resources" "Contents/Plugins")

for folder in "${folders[@]}"
do
  link=$app/$folder
  reltarget="$(readlink $link)"

  if [ -n "$reltarget" ]; then
    rm -f "$link"
    cp -af "$reltarget" "$link" || {
        # on failure, restore the symlink
        rm -rf "$link"
        ln -sf "$reltarget" "$link"
    }
  fi
done

mkdir -p $app/Contents/Resources/bin
cp -rf $app/../game.app/Contents/MacOS/game $app/Contents/Resources/bin/

cef_helper_dir="$app/Contents/Frameworks/cef.helper.app"

if [ -d "$cef_helper_dir" ]; then
  rm -rf "$cef_helper_dir/Contents/Frameworks"
  mkdir "$cef_helper_dir/Contents/Frameworks"
  ln -sf "../../../Chromium Embedded Framework.framework" "$cef_helper_dir/Contents/Frameworks/Chromium Embedded Framework.framework"
fi
