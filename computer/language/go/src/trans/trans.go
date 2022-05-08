package main

import (
        "fmt"
	"os"
	"bufio"
	"time"
	"strings"
	"unicode/utf8"
	"io/ioutil"
	"log"

	"gopkg.in/yaml.v2"
        "github.com/tencentcloud/tencentcloud-sdk-go/tencentcloud/common"
        "github.com/tencentcloud/tencentcloud-sdk-go/tencentcloud/common/errors"
        "github.com/tencentcloud/tencentcloud-sdk-go/tencentcloud/common/profile"
        tmt "github.com/tencentcloud/tencentcloud-sdk-go/tencentcloud/tmt/v20180321"
)

var client *tmt.Client
type TextLine struct {
    ZH string
    EN string
    StartTime int64
    EndTime int64
}

type Conf struct {
    AppID string `yaml:"appid"`
    AppKey string `yaml:"appkey"`
}

func init() {
	yamlFile, err := ioutil.ReadFile("trans_conf.yaml")
	if err != nil {
	    log.Fatalf("read conf fail", err)
	}
	conf := &Conf{}
	err = yaml.Unmarshal(yamlFile, conf)
	if err != nil {
	    log.Fatalf("Unmarshal yaml fail: %v", err)
	}
	if conf.AppID == "" || conf.AppKey == "" {
	    log.Fatalf("missing conf")
	}
        credential := common.NewCredential(
		conf.AppID,
		conf.AppKey,
        )
        cpf := profile.NewClientProfile()
        cpf.HttpProfile.Endpoint = "tmt.tencentcloudapi.com"
        client, _ = tmt.NewClient(credential, "ap-guangzhou", cpf)
}

func Split(r rune) bool {
    return r == ':' || r == '.' || r == ','
}

func SplitZh(r rune) bool {
    return r == '，' || r == '。'
}

func num(str string) int {
    num := 0
    for _, v := range str {
	num = num * 10 + (int(v) - '0')
    }
    return num
}

//[0:0.080,0:46.400] 
func converTimeTag(timeTag string) (int64, int64) {
    fmt.Println(timeTag)
    trimTimeTag := strings.TrimSpace(timeTag)
    tags := strings.FieldsFunc(trimTimeTag[1:len(trimTimeTag)-1], Split)
    //fmt.Println(tags)
    start := num(tags[0]) * 60000 + num(tags[1])*1000 + num(tags[2])
    end := num(tags[3]) * 60000 + num(tags[4])*1000 + num(tags[5])
    return int64(start), int64(end)
}

func genSrt(oldTextLine []*TextLine) (newTextLine []*TextLine) {
    //开篇
    newTextLine = append(newTextLine, &TextLine{
		ZH: "声明：本字幕由\"啊呜\"使用机器学习,自动翻译\nFFmpeg等工具合成,仅供观赏，不得用于商业用途！",
		EN: "copyright:啊呜",
		EndTime: 5000,
    })
    
    for _, line := range oldTextLine {
		// 总时间
		sum := line.EndTime - line.StartTime	
		// 总字数
		sumChar := utf8.RuneCountInString(line.ZH)
		// 标点断句
		dots := strings.FieldsFunc(line.ZH, SplitZh)
		// 每个字的时间
		eachCharTime := int(sum)/(sumChar - len(dots))
		// 初始化
		start := line.StartTime
		end := int64(0)
		currDotLen := 0
		currDot := ""
		// 遍历短句
		for _, dot := range dots {
			// 短句的长度
			dot = strings.TrimSpace(dot)
			dotLen := utf8.RuneCountInString(dot)
			if dotLen == 0 { // 空白行，跳过
				continue
			} 
			// 尝试累加短句，长度不超过32, 累加
			if currDotLen + dotLen < 32 {
				if currDotLen == 0 {
					currDot = dot // 第一句
				} else {
					currDot = fmt.Sprintf("%s, %s", currDot, dot) // 使用逗号分割
				}
				currDotLen += dotLen
				continue //继续累加
			} else {  // 累加超长，先把旧的写入
				if currDotLen == 0 {
					currDot = dot
					currDotLen = dotLen
					continue
				}

				end = start + int64(currDotLen * eachCharTime)
				tl := &TextLine {
					StartTime: start,
					EndTime: end,
				}
				// 过长拆分两行
				if currDotLen > 32 {
					zhStr := []rune(currDot)
					if zhStr[currDotLen/2] == ','{ // 卡好分割点是标点，提到前半句
						tl.ZH = fmt.Sprintf("%s\n%s", string(zhStr[:currDotLen/2+1]), string(zhStr[currDotLen/2+1:]))
					} else {
						tl.ZH = fmt.Sprintf("%s\n%s", string(zhStr[:currDotLen/2]), string(zhStr[currDotLen/2:]))
					}
				} else {
					tl.ZH = currDot
				}
				newTextLine = append(newTextLine, tl)

				// 重新累加
				currDot = dot
				currDotLen = dotLen
			} 
			start = end + 100
		}

		// 此段落结束，因为每个段落的语速不同，时间差不同，不跨段整合，这里写出剩余部分
		if currDotLen != 0 {
			newTextLine = append(newTextLine, &TextLine {
				ZH: currDot,
				StartTime: start,
				EndTime: start + int64(currDotLen * eachCharTime),
			})
		}
		// 修订最后时间
		newTextLine[len(newTextLine)-1].EndTime = line.EndTime
    }

    //结束
    end := newTextLine[len(newTextLine)-1].EndTime
    newTextLine = append(newTextLine, &TextLine{
		ZH: "谢谢观赏，请点赞关注！",
		EN: "copyright:啊呜",
		StartTime: end - int64(5000),
		EndTime: end,
    })
    
    return
}

func genTimeTag(ms int64) string {
    timeStr := time.Unix(ms/1000, 0).Format("15:04:05")
    t, _ := time.ParseInLocation("15:04:05", timeStr, time.Local)
    return fmt.Sprintf("%s,%d", time.Unix(t.Unix() - 28800, 0).Format("15:04:05"), ms % 1000)
}

func writeSrt(filePath string, textLine []*TextLine) error {
    srt, err := os.Create(filePath + ".srt")
    if err != nil {
	return err
    }
    defer srt.Close()
    for i, tl := range textLine {
	// 组合字幕
	ziMu := fmt.Sprintf("%d\n%s --> %s\n%s\n\n",
		i+1,
		genTimeTag(tl.StartTime), genTimeTag(tl.EndTime),
		tl.ZH)
	_, err := srt.WriteString(ziMu)
	if err != nil {
	    return err
	}
    }
    return nil
}

func main() {
    if len(os.Args) != 3 {
	    fmt.Println("缺少参数: ./trans input output")
	    return
	}
        request := tmt.NewTextTranslateRequest()
        request.Source = common.StringPtr("en")
        request.Target = common.StringPtr("zh")
        request.ProjectId = common.Int64Ptr(0)
        
	file, err := os.Open(os.Args[1])
	if err != nil {
	    fmt.Printf("open file fail, %v", err)
	    return
	}
	defer file.Close()

	scanner := bufio.NewScanner(file)
	var textLine []*TextLine
	for scanner.Scan() {
	    // fmt.Println(scanner.Text())
	    tl := &TextLine{}
	    src := scanner.Text()
	    idx := strings.Index(src, "]  ")
	    tl.StartTime, tl.EndTime = converTimeTag(src[:idx+2])
	    tl.EN = src[idx+2:]

	    request.SourceText = common.StringPtr(src[idx+2:])
	    response, err := client.TextTranslate(request)
	    if _, ok := err.(*errors.TencentCloudSDKError); ok {
		    fmt.Printf("An API error has returned: %s", err)
		    return
	    }
	    if err != nil {
		    panic(err)
	    }
	    //fmt.Printf("%s", response.ToJsonString())
	    
	    fmt.Println(*response.Response.TargetText)
	    tl.ZH = *response.Response.TargetText
	    textLine = append(textLine, tl)
	    time.Sleep(250 * time.Millisecond)
	}

	if err := scanner.Err(); err != nil {
	    fmt.Printf("open file fail, %v", err)
	    return
	}

	srtTextLine := genSrt(textLine)
	for _, srt := range srtTextLine {
	    fmt.Printf("result: %v", *srt)
	}

	err = writeSrt(os.Args[2], srtTextLine)
	if err != nil {
	    fmt.Printf("写文件失败:%v", err)
	}
	fmt.Println("处理完成！")
} 
