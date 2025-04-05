# QueryCraft

简易的 MySQL 图形化界面，支持单表查询和联合查询

`res`目录下存放了数据库连接所需的动态链接库，其中`qsqlmysql.dll`适配 Qt 5.14.2，来自[压缩包](https://github.com/thecodemonkey86/qt_mysql_driver/files/4460982/qsqlmysql.dll_Qt_SQL_driver_5.14.2_MinGW_64-Bit.zip)的`debug/sqldrivers`目录下；剩下三个`.dll`文件都来自[压缩包](https://github.com/thecodemonkey86/qt_mysql_driver/releases/download/qmysql_6.9.0/qsqlmysql.dll_Qt_SQL_driver_6.9.0_MinGW_13.1_64-bit.zip)

请修改`CMakeLists.txt`中的`CMAKE_PREFIX_PATH`为你的 Qt 安装路径，如果你安装的版本不是 5.14.2，请换用相应版本的`qsqlmysql.dll`
