del /s *.txt
del *.zip
del propsfile
IF exist Debug (
        rd/s/q  Debug
    )
IF exist Release (
        rd/s/q  Release
    )