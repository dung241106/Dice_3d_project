"""Package source, Windows runtime and presentation materials for submission."""
from pathlib import Path
from zipfile import ZipFile, ZIP_DEFLATED

root = Path(__file__).resolve().parents[1]
output = root.parent / (root.name + '_NopBai.zip')
runtime = {'Dice3D_Edu.exe', 'libfreeglut.dll', 'libwinpthread-1.dll'}
count = 0
with ZipFile(output, 'w', ZIP_DEFLATED, compresslevel=6) as archive:
    for file in sorted(root.rglob('*')):
        if not file.is_file():
            continue
        relative = file.relative_to(root)
        if relative.parts[0] in {'tmp', 'captures', '.git'}:
            continue
        if relative.parts[:2] == ('docs', 'slide_previews'):
            continue
        if relative.parts[0] == 'build' and (len(relative.parts) != 2 or file.name not in runtime):
            continue
        archive.write(file, Path(root.name) / relative)
        count += 1
print(f'Packaged {count} files, {output.stat().st_size / 1024 / 1024:.2f} MiB')
print(output)
