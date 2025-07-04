#include "itrayloadplugin.h"

// 析构函数必须在cpp中实现以确保正确导出
ITrayLoadPlugin::~ITrayLoadPlugin() = default;

// 确保moc能正确生成元对象代码
#include "moc_itrayloadplugin.cpp"
