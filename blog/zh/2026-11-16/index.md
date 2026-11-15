---
layout: post
title: 示例文章：博客使用说明
date: 2026-11-16
lang: zh
---

这是一篇示例文章，用于演示博客的写作方式，可直接删除或改写。

## 如何写一篇文章

1. 中文文章放在 `blog/zh/` 下，英文文章放在 `blog/en/` 下，各新建一个以日期命名的目录，如 `blog/zh/2026-11-16/`；
2. 在目录内创建 `index.md`，开头写 front matter：

   ```yaml
   ---
   layout: post
   title: 文章标题
   date: 2026-11-16
   lang: zh          # 中文文章 zh，英文文章 en
   ---
   ```

3. 之后用 Markdown 正文写作即可，文章会按语言自动出现在[中文博客目录](/blog/)或[英文博客目录](/blog/en/)中。

## 在文章中引用图片

图片等资源直接放在文章目录内（如 `images/` 子目录），用相对路径引用：

![示例图片](images/placeholder.svg)

## Markdown 能力

支持 GitHub 风格 Markdown：表格、任务列表、删除线、代码高亮等。

```python
from hikyuu.interactive import *

my_tm = crtTM(init_cash=300000)
my_sg = SG_Flex(EMA(CLOSE(), n=5), slow_n=10)
my_mm = MM_FixedCount(1000)
sys = SYS_Simple(tm=my_tm, sg=my_sg, mm=my_mm)
sys.run(sm['sz000001'], Query(-150))
```

| 表格 | 示例 |
| ---- | ---- |
| A    | 1    |
| B    | 2    |
