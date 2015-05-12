# xmodem
embedded

主要实现的功能：
    xmodem支持
    xmodem-1K支持
    高度可移植性，只需提供两个底层驱动函数
        XMODEM_WRITE_BYTE(__BYTE)
        XMODEM_READ_BYTE(__BYTE)
    配置文件，实现对xmodem的灵活控制