<h5>作者：徐金光  日期：2020年8月29日</h5>

[TOC]


## 0.URL和URI的定义

### 0.1.URL

URL=**U**niform **R**esource **L**ocators 的缩写。在1994年的[RFC1738](https://www.ietf.org/rfc/rfc1738.txt)标准文件中规范URL的语法和语义。用于定位和访问网络资源。

URL语法的完整BNF描述

```shell
URL:
		<scheme>:<scheme-specific-part>
scheme-specific-part:
		//<user>:<password>@<host>:<port>/<url-path>
user:
		[optional]
password:
		[optional]
host:
		domain-name | ip
port:
		[optional]
```

- scheme: 有字母，数字，-，+，点构成，大小写不敏感

  - http 超文本传输协议
  - ftp文本传输协议
  - gopher协议
  - mailto邮件协议
  - Usenet
  - Nntp
  - telnet
  - wais
  - file
  - prospero

- user / password 中如果出现这三个 : / @字符都需要编码。不能只有password没有user

- port 如果没有指定，则使用对应协议的默认端口

- url-path 和前面host(port)之间的/不是它的一部分。路径格式因协议存在差异

这里看看常用的http URL
```shell
http-url:
		http://<host>:<port>/<path>?<search>
port:
		[80 | 8080 | 自定义端口数值]
path:
		hsegment *[ "/" hsegment ]
hsegment:
		*[ uchar | ";" | ":" | "@" | "&" | "=" ]
search:
		*[ uchar | ";" | ":" | "@" | "&" | "=" ]
```

`*`正则表达式的出现0~n次。

字符集语法

```shell
lowalpha       = "a" | "b" | "c" | "d" | "e" | "f" | "g" | "h" |
                 "i" | "j" | "k" | "l" | "m" | "n" | "o" | "p" |
                 "q" | "r" | "s" | "t" | "u" | "v" | "w" | "x" |
                 "y" | "z"
hialpha        = "A" | "B" | "C" | "D" | "E" | "F" | "G" | "H" | "I" |
                 "J" | "K" | "L" | "M" | "N" | "O" | "P" | "Q" | "R" |
                 "S" | "T" | "U" | "V" | "W" | "X" | "Y" | "Z"
alpha          = lowalpha | hialpha
digit          = "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9"

safe           = "$" | "-" | "_" | "." | "+"
extra          = "!" | "*" | "'" | "(" | ")" | ","

national       = "{" | "}" | "|" | "\" | "^" | "~" | "[" | "]" | "`"
punctuation    = "<" | ">" | "#" | "%" | "

reserved       = ";" | "/" | "?" | ":" | "@" | "&" | "="

hex            = digit | "A" | "B" | "C" | "D" | "E" | "F" |
                 "a" | "b" | "c" | "d" | "e" | "f"
escape         = "%" hex hex

unreserved     = alpha | digit | safe | extra
uchar          = unreserved | escape
xchar          = unreserved | reserved | escape
digits         = 1*digit
```



### 0.2. URI

URI=**U**niform **R**esource **I**dentifier 2005年[RFC3986](https://tools.ietf.org/html/rfc3986)标准文档中规范，此文档整合了上面的URL部分以及其他历史文档形成适合新时代的规范。可以理解成统一版。因此，**URL只是URI的一个子集**

> A URI can be further classified as a locator, a name, or both.  The
>    term "Uniform Resource Locator" (URL) refers to the subset of URIs
>    that, in addition to identifying a resource, provide a means of
>    locating the resource by describing its primary access mechanism
>    (e.g., its network "location").  The term "Uniform Resource Name"
>    (URN) has been used historically to refer to both URIs under the
>    "urn" scheme [RFC2141], which are required to remain globally unique
>    and persistent even when the resource ceases to exist or becomes
>    unavailable, and to any other URI with the properties of a name.

以后统一称为URI。

URI语法如下

```shell
	 URI           = scheme ":" hier-part [ "?" query ] [ "#" fragment ]

   hier-part     = "//" authority path-abempty
                 / path-absolute
                 / path-rootless
                 / path-empty

   URI-reference = URI / relative-ref

   absolute-URI  = scheme ":" hier-part [ "?" query ]

   relative-ref  = relative-part [ "?" query ] [ "#" fragment ]

   relative-part = "//" authority path-abempty
                 / path-absolute
                 / path-noscheme
                 / path-empty

   scheme        = ALPHA *( ALPHA / DIGIT / "+" / "-" / "." )

   authority     = [ userinfo "@" ] host [ ":" port ]
   userinfo      = *( unreserved / pct-encoded / sub-delims / ":" )
   host          = IP-literal / IPv4address / reg-name
   port          = *DIGIT

   IP-literal    = "[" ( IPv6address / IPvFuture  ) "]"

   IPvFuture     = "v" 1*HEXDIG "." 1*( unreserved / sub-delims / ":" )

   IPv6address   =                            6( h16 ":" ) ls32
                 /                       "::" 5( h16 ":" ) ls32
                 / [               h16 ] "::" 4( h16 ":" ) ls32
                 / [ *1( h16 ":" ) h16 ] "::" 3( h16 ":" ) ls32
                 / [ *2( h16 ":" ) h16 ] "::" 2( h16 ":" ) ls32
                 / [ *3( h16 ":" ) h16 ] "::"    h16 ":"   ls32
                 / [ *4( h16 ":" ) h16 ] "::"              ls32
                 / [ *5( h16 ":" ) h16 ] "::"              h16
                 / [ *6( h16 ":" ) h16 ] "::"

   h16           = 1*4HEXDIG
   ls32          = ( h16 ":" h16 ) / IPv4address
   IPv4address   = dec-octet "." dec-octet "." dec-octet "." dec-octet

   dec-octet     = DIGIT                 ; 0-9
                 / %x31-39 DIGIT         ; 10-99
                 / "1" 2DIGIT            ; 100-199
                 / "2" %x30-34 DIGIT     ; 200-249
                 / "25" %x30-35          ; 250-255

   reg-name      = *( unreserved / pct-encoded / sub-delims )

   path          = path-abempty    ; begins with "/" or is empty
                 / path-absolute   ; begins with "/" but not "//"
                 / path-noscheme   ; begins with a non-colon segment
                 / path-rootless   ; begins with a segment
                 / path-empty      ; zero characters

   path-abempty  = *( "/" segment )
   path-absolute = "/" [ segment-nz *( "/" segment ) ]
   path-noscheme = segment-nz-nc *( "/" segment )
   path-rootless = segment-nz *( "/" segment )
   path-empty    = 0<pchar>

   segment       = *pchar
   segment-nz    = 1*pchar
   segment-nz-nc = 1*( unreserved / pct-encoded / sub-delims / "@" )
                 ; non-zero-length segment without any colon ":"

   pchar         = unreserved / pct-encoded / sub-delims / ":" / "@"

   query         = *( pchar / "/" / "?" )

   fragment      = *( pchar / "/" / "?" )

   pct-encoded   = "%" HEXDIG HEXDIG

   unreserved    = ALPHA / DIGIT / "-" / "." / "_" / "~"
   reserved      = gen-delims / sub-delims
   gen-delims    = ":" / "/" / "?" / "#" / "[" / "]" / "@"
   sub-delims    = "!" / "$" / "&" / "'" / "(" / ")"
                 / "*" / "+" / "," / ";" / "="
```

和上文对比一下，会发现，它多了很多内容，而且定义的更加准确和宽泛。

----

## 1. 为什么要编码

### 1.1 URL

URL使用ASCII字符集，目前浏览器都是用的utf-8编码。和ASCII兼容的。但是根据标准文档URL有三类字符，这些字符尤其特别含义

- 不安全字符 < > # % {} | \ ^ ~ [] `

- 保留字符 ；/ ? :  @  & = 

- 特殊字符  $ - _ . + ! * ' ( ) , 

因此，这些字符作为用户使用的时候不能直接使用，需要转码，避免和URL自身歧义。编码方式是 `% hex hex`

### 1.2. URI

URI和URL相比，将特殊字符统一成了保留字符。

```shell
   unreserved    = ALPHA / DIGIT / "-" / "." / "_" / "~"
   reserved      = gen-delims / sub-delims
   gen-delims    = ":" / "/" / "?" / "#" / "[" / "]" / "@"
   sub-delims    = "!" / "$" / "&" / "'" / "(" / ")"
                 / "*" / "+" / "," / ";" / "="
```

### 1.3. 编码值

- 空格   -   %20
- "      -   %22
- \#     -   %23
- %     -   %25
- &     -   %26
- (      -   %28
- )      -   %29
- \+     -   %2B
- ,      -   %2C
- /      -   %2F
- :      -   %3A
- ;      -   %3B
- <     -   %3C
- =     -   %3D
- \>     -   %3E
- ?     -   %3F
- @    -   %40
- \      -   %5C
- |      -   %7C 

## 3. 以URI为准

以上只是参考了历史的URL标准，应该以最新的URI为标准。给出URL只是作为对比来看。

使用规则：

1. 空格换成加号(+)
2. 正斜杠(/)分隔目录和子目录
3. 问号(?)分隔URL和查询
4. 百分号(%)制定特殊字符
5. \#号指定书签
6. &号分隔参数

## 参考资料

1. [关于url编码-阮一峰](http://www.ruanyifeng.com/blog/2010/02/url_encoding.html)
2. [RFC1738](https://www.ietf.org/rfc/rfc1738.txt)
3. [RFC3986](https://tools.ietf.org/html/rfc3986)
4. [网址URL中特殊字符转义编码](https://blog.csdn.net/pcyph/article/details/45010609)


