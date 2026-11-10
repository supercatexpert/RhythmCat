# Translations

The core library and both frontends are translated (gettext domain
`rhythmcat3`, in `po/`) into:

* Chinese (simplified), `zh_CN`
* Chinese (traditional), `zh_TW`
* Japanese, `ja`

The language follows the locale (`LANG`, `LC_ALL` or `LANGUAGE`), for
example `LANG=ja_JP.UTF-8 rhythmcat`. The translations are installed to
`share/locale`, so the program must be installed to use them. The name,
generic name, comment and keywords in the desktop file are translated too.

To update the translations after changing the strings:

```sh
ninja -C build rhythmcat3-pot        # regenerate po/rhythmcat3.pot
ninja -C build rhythmcat3-update-po  # merge it into the .po files
```

To add a language, add its code to `po/LINGUAS` and create the `.po` file
from the template (for example with `msginit`).
