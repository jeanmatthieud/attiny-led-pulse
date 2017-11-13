# For 1MHz
Import('env')
env.Replace(FUSESCMD="avrdude $UPLOADERFLAGS -e -Ulfuse:w:0x62:m -Uhfuse:w:0xDF:m -Uefuse:w:0xFF:m")
