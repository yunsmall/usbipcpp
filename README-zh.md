# usbipcpp

一个用于创建 usbip 服务器的 C++ 库

> **项目状态**：早期开发阶段  
> ✅ 已在 Linux 平台实现基于 libusb 的 usbip 服务器功能  
> ⚠️ **Windows 平台暂不可用**：控制传输存在阻塞问题，欢迎提供解决方案

## 项目目标
- 支持动态添加虚拟 USB 设备
- 实现网络共享 USB 设备功能
- 完善 USB 协议栈支持（因协议复杂性，需持续开发）

欢迎贡献代码！🚀

---

## 编译指南
```bash
cmake -B build
cmake --build build
cmake --install build
```

## 感谢 
非常感谢下面的项目，我从中学到了很多。在本项目中甚至能看到相似的代码:  
- [usbipd-libusb](https://github.com/raydudu/usbipd-libusb)  
- [usbip](https://github.com/jiegec/usbip)  