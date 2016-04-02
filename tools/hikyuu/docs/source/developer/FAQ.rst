常见问题
========

如何制作PDF文档
---------------

参考： `《用Python做科学计算》 <http://hyry.dip.jp/pydoc/pydoc_write_tools.html>`_

调用make latex命令可以输出为latex格式的文件，然后调用 xelatex scipydoc.tex 即可将其转换为PDF文件，xelatex是proTeXt自带的命令。制作PDF文档时同样有中文无法显示的问题，按照以下步骤解决:

::

   latex_preamble = r"""
       \usepackage{float}
       \textwidth 6.5in
       \oddsidemargin -0.2in
       \evensidemargin -0.2in
       \usepackage{ccaption}
       \usepackage{fontspec,xunicode,xltxtra}
       \setsansfont{WenQuanYi Micro Hei}
       \setromanfont{WenQuanYi Micro Hei}
       \setmainfont{WenQuanYi Micro Hei}
       \setmonofont{WenQuanYi Zen Hei Mono}
       \XeTeXlinebreaklocale "zh"
       \XeTeXlinebreakskip = 0pt plus 1pt
       \renewcommand{\baselinestretch}{1.3}
       \setcounter{tocdepth}{3}
       \captiontitlefont{\small\sffamily}
       \captiondelim{ - }
       \renewcommand\today{\number\year年\number\month月\number\day日}
       \makeatletter
       \renewcommand*\l@subsection{\@dottedtocline{2}{2.0em}{4.0em}}
       \renewcommand*\l@subsubsection{\@dottedtocline{3}{3em}{5em}}
       \makeatother
       \titleformat{\chapter}[display]
       {\bfseries\Huge}
       {\filleft \Huge 第 \hspace{2 mm} \thechapter \hspace{4 mm} 章}
       {4ex}
       {\titlerule
       \vspace{2ex}%
       \filright}
       [\vspace{2ex}%
       \titlerule]
       %\definecolor{VerbatimBorderColor}{rgb}{0.2,0.2,0.2}
       \definecolor{VerbatimColor}{rgb}{0.95,0.95,0.95}
   """.decode("utf-8")

进入build/latex目录，运行下面的命令输出PDF文档，使用nonstopmode，即使出现错误也不暂停运行。

::

  > xelatex -interaction=nonstopmode Hikyuu.tex

如何去除
d:\workspace\fasiondog\trunk\libs\galaxy\galaxy\tradesys\cost\../TradeCostBase.h : warning C4819: 该文件包含
不能在当前代码页(936)中表示的字符。请将该文件保存为 Unicode 格式以防止数据丢失
rule configure-version-specific ( toolset : version : conditions )
{
    toolset.push-checking-for-flags-module unchecked ;
    # Starting with versions 7.0, the msvc compiler have the /Zc:forScope and
    # /Zc:wchar_t options that improve C++ standard conformance, but those
    # options are off by default. If we are sure that the msvc version is at
    # 7.*, add those options explicitly. We can be sure either if user specified
    # version 7.* explicitly or if we auto-detected the version ourselves.
    if ! [ MATCH ^(6\\.) : $(version) ]
    {
        toolset.flags $(toolset).compile CFLAGS $(conditions) : /Zc:forScope /Zc:wchar_t ;
        toolset.flags $(toolset).compile.c++ C++FLAGS $(conditions) : /wd4675 ;
        toolset.flags $(toolset).compile.c++ C++FLAGS $(conditions) : /wd4819 ;

b2 -j 4 release link=shared address-model=64