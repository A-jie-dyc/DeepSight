## 项目一

技术栈：C++17、Qt6、TCP网络编程、SQLite、Git

项目描述：
独立设计并开发C/S架构即时通讯客户端，全程主导架构设计、编码实现与Bug调试，聚焦聊天通信与大文件传输核心能力落地：

网络通信核心开发：基于TCP Socket搭建长连接通信框架，自定义二进制协议——魔数0x5A5A5A5A帧头标识+7字节定长头部+变长负载，字节级逐位重同步处理粘包拆包与脏数据；5秒心跳保活+连续3次超时断连+指数退避重连（2s→32s，最多8次），接收端100MB缓冲区上限防恶意数据洪泛。

程序架构解耦设计：moveToThread将QTcpSocket及全部网络I/O移入独立工作线程，UI通过QMetaObject::invokeMethod跨线程调度，Worker通过信号回传数据至主线程，彻底解耦UI与网络逻辑。

大文件传输优化：设计Query/Resp两阶段协商协议实现断点续传，64KB流式分块+bytesWritten串行流控；超大文件采用QFileDevice::map内存映射直接写入虚拟内存，避免频繁seek/write开销，中断后自动从断点恢复。

本地聊天记录持久：集成SQLite，设计chat_history表存储设备标识、发送方、消息内容与时间戳，支持按设备级会话查询与清空删除，参数化查询防注入。

工程化实践落地：分类日志模块（连接/断开/重连/心跳/文件传输/异常），带时间戳展示于日志窗口；Git版本管理，规范提交记录，完成多轮Bug修复与稳定性迭代。

项目地址：https://github.com/A-jie-dyc/IM_Client

---

## 项目二

技术栈：C++17、Qt6 Quick、OpenCV、ONNX Runtime、CMake

项目描述：
独立设计并开发智能人流统计桌面应用，集成YOLOv8n+ONNX Runtime实现实时人员检测、多目标跟踪与越线计数：

AI推理管线与多线程架构：设计7线程流水线（采集→预处理→ONNX推理→NMS后处理→IOU跟踪→绘制→计数），moveToThread+QueuedConnection解耦各模块，uint64_t frameId串联帧数据；原子变量CAS忙丢策略防止队列堆积，Sequence Lock无锁FPS计数器消除锁竞争。

预处理与后处理：Letterbox等比例缩放+灰边填充至640×640，cv::dnn::blobFromImage归一化输出；后处理置信度过滤+NMS去重后，通过scale/pad参数将检测坐标逆映射回原始分辨率。

检测框可视化与帧缓存：QPainter绘制红色边界框、ID标签（半透明圆角背景）与绿色越线统计线；FrameId哈希表缓存机制，推导延迟帧数公式动态扩容，三级备选匹配策略（精确→最近旧帧→最旧可用帧）保证高频采集下不丢帧。

越线计数与QML交互：QQuickPaintedItem支持鼠标拖拽绘制统计线，叉积法实时判断行人跨越方向并去重计数；Q_PROPERTY+信号驱动QML界面实时刷新，支持运行时启停AI检测与切换视频源，FPS浮层实时展示推理/输入帧率与丢帧率。

工程化与异常处理：分类四级错误类型×三级严重度，Controller统一收集工作线程异常并中文化错误描述与恢复建议；ONNX预分配输入缓冲区，推理线程数按CPU核心自适应；Debug构建启用AddressSanitizer。
