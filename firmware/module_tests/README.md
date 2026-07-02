# 单模块测试工程

每个模块单独建一个测试工程，跑通后再合入主控工程。

建议目录：

```text
servo_test/
hx711_test/
max30102_test/
wifi_test/
qmi8658_test/
lcd_test/
```

测试工程只保留关键源码和配置，不提交 `Objects/`、`Listings/`、`Debug/` 等编译输出。
