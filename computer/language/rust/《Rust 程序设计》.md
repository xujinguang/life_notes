# 《Rust 程序设计语言笔记》

2021-05-09

书籍：https://kaisery.github.io/trpl-zh-cn

## 第2章

1. *Cargo.toml* 文件决定程序的依赖库
2. Cargo.lock 文件确保构建是可重现的
3. `Rng` 是一个 trait
4. Result 枚举类型，成员Err和Ok
5. Ordering 枚举类型，成员是 `Less`、`Greater` 和 `Equal`
6. Rust 有一个静态强类型系统
7. 不过 Rust 允许用一个新值来 **隐藏** （*shadow*） `guess` 之前的值

```rust
//使用标准输入输出库
use std::io;
//外部依赖，在toml中添加
use rand::Rng;
//标准库的比较，Ordering是枚举
use std::cmp::Ordering;

fn main() {
	println!("guess number game!");
	println!("Please input your guess.");

	//生成一个随机数
	let secret_number = 
		//当前执行线程的本地环境中，并从操作系统获取 seed
		rand::thread_rng()
		//包含下限但不包含上限
		.gen_range(1, 101);
	//println!("The secret number is: {}", secret_number);

	loop{
		//定义一个可变变量并绑定到String类创建的实例
		let mut guess = String::new(); 

		//使用标准输入方法读取输入内容存储到变量guess，并返回一个io::Result实例
		io::stdin().read_line(&mut guess)
			//Reuslt类型是枚举，成员包括Err和Ok，Ok标识成功包括成功的结果，Err标识失败包括失败结果
			//Result实例包括expect方法，如果返回Err则expect导致程序崩溃，并显示expect的参数内容
			.expect("Failed to read line");
		//{}是占位符
		println!("You guessed: {}", guess);
		//转换成数值
		let guess: u32 = match guess
			//去除字符串两端的空白
			.trim()
			//字符串解析成数值，类型在u32时指定
			.parse() {
				//返回Result类型
				//.expect("Please type a number!");
				Ok(num) => num, //num是实际的值，返回
					Err(_) => continue, //_通配符，继续循环
			};

		//比较，并匹配结论
		match guess.cmp(&secret_number) {
			Ordering::Less => println!("Too small!"),
				Ordering::Greater => println!("Too big!"),
				Ordering::Equal => {
					println!("You win!");
					break;//退出循环
				}
		}
	}
}

```



## 第3章 编程概念

1. 变量的可变性不可变性，不可变不同于常量