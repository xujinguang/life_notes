package main

import (
	"fmt"
	"reflect"
	)

// 类型
type BaseFace interface{
	Sleep() string
	WakeUp() error
	Dream() string
}

// 实例
type Women struct {
	Age int
	Name string
}

type Man struct {
	Age int
	Weight int
}

// 实现类型中的方法
func (w *Women) Sleep() string {
	fmt.Println(w.Name)
	fmt.Println("Go to sleep!")
	return "nothing"
}

func (w *Women) WakeUp() error {
	fmt.Println(w.Name)
	fmt.Println("hello world")
	return nil
}

func (w *Women) Dream() string {
	fmt.Println(w.Name)
	fmt.Println("color")
	return "sound"
}

// 自己独有的方法
func (w *Women) Dance() {
	fmt.Println("fly")
}

func (w *Women) Song() {
}

func (m *Man) Dance() {
}

func (m *Man) Sleep() string {
	return "nothing"
}

func (m *Man) WakeUp() error {
	return nil
}

func (m *Man) Dream() string {
	return "sound"
}

func Night(face BaseFace) {
	// 在BaseFace的基础上增加方法
	// w, ok := face.(*Women) //这样最直接的方式
	w, ok := face.(interface{Dance()}) // 明显方式转换
	if ok {
		fmt.Println(reflect.TypeOf(w))
		w.Dance()
	}
	// face.Dance() // 直接调用是错误的，BaseFace明确没有这个方法
}

func CreateWomen() BaseFace {
	return &Women {
		Age : 29,
		Name : "yang",
	}
}

func CreateMan() BaseFace {
	return &Man {
	}
}
func main() {
	w := CreateWomen()
	Night(w)
	m := CreateMan()
	Night(m)
}
