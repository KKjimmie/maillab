# maillab
> HITsz Computer Network Lab9, Spring 2023
#### 介绍
邮件客户端实验代码框架

send.c为邮件发送客户端源程序，recv.c为邮件接收客户端源程序。
首先补充文件，在适当位置填上账号密码。
执行make编译后，会生成send和recv两个程序。send程序需要接收命令行参数，它的使用方法为：

```
./send [-s SUBJECT] [-m MESSAGE] [-a ATTACHMENT] RECIPIENT
```

SUBJECT: 邮件主题
MESSAGE: 邮件正文或含有邮件正文的文件路径
ATTACHMENT: 邮件附件
RECIPIENT: 收件人地址

recv不需要任何参数，直接执行即可。