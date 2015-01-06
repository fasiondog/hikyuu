/*
 * doc.h
 *
 *  Created on: 2010-5-27
 *      Author: fasiondog
 */

#ifndef DOC_H_
#define DOC_H_

/******************************************************************************
 * 该文件仅用于doxygen生成文档
 *****************************************************************************/

/**
 * 常用的独立工具集
 * @defgroup Utilities Utilities 常用独立工具集
 *
 * @details
 * 设计准则：
 *     @arg 每个工具模块彼此独立不互相依赖，并且不依赖于fsd空间下
 *          的其他模块。\n
 *     @arg 保持这种独立性，便于可以独立拆分移植或在其他程序中使用。\n
 *     @arg 除Boost库外，各模块尽可能不包含其他外部依赖的库。如果存在外部依赖，
 *          将予以相应的说明。\n
 *
 * @note 这里把Boost库作为C++的基础库使用，可能严重依赖于Boost库，移植时需要注意
 *
 * @defgroup IniParser IniParser 简单ini文件读取
 * @details 只支持简单的Win格式的ini文件
 * @ingroup Utilities
 */

#endif /* DOC_H_ */
