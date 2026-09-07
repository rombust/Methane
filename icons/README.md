# Icons

Source artwork for the application icon. Nothing here is opened by the game at
run time, so none of it needs installing as game data.

| File | Used by |
| --- | --- |
| `icon.ico` | `Sources/Game/methane.rc`, compiled into `methane.exe` by the Windows build |
| `icon24x24.png` | icon theme installs |
| `icon48x48.png` | icon theme installs |
| `icon128x128.png` | icon theme installs |
| `icon256x256.png` | icon theme installs |

## The three that live elsewhere

`resources/` holds `icon16x16.png`, `icon32x32.png` and `icon64x64.png`. 

## Packaging on Linux

A distribution package would install the sizes above, plus the three in
`resources/`, under `share/icons/hicolor/<size>x<size>/apps/methane.png`, and
add a `.desktop` file pointing at them. 

