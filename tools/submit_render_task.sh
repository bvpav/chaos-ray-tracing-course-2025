#!/bin/sh --

if [ $# != 2 ]; then
    echo "usage: $0 [<task-slug>] [<task-name>]" >&1
    exit 1
fi

task_slug="$1"
task_name="$2"

cmake --build build/ || exit

printf '# %s\n\n' "$task_name" > src/README.md

table="| $task_name | [src/](https://github.com/bvpav/chaos-ray-tracing-course-2025/tree/$task_slug/src) | "
br=

for scene_path in "scenes/$task_slug/"*.crtscene; do
    scene_name="$(basename "$scene_path")"
    scene_name="${scene_name%.crtscene}"
    ppm_path="results/ppm/$task_slug-$scene_name.ppm"
    png_path="results/png/$task_slug-$scene_name.png"

    echo "Rendering $scene_path..."
    build/crt_renderer "$scene_path" "$ppm_path" || exit
    magick "$ppm_path" "$png_path" || exit

    git add "$ppm_path" "$png_path"

    echo "[![$task_name: $scene_name](../$png_path)](../$ppm_path)" >> src/README.md

    table="$table${br}[![$task_name: $scene_name]($png_path)]($ppm_path)"
    br="<br>"
done

echo "$table |" >> README.md

printf 'Press ENTER when done editing...'
read -r _

git add README.md src/README.md
git commit -m "feat: Complete $task_name" && git tag "$task_slug" || exit
git push && git push origin "$task_slug" || exit