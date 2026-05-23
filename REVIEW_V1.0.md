# V1.0 代码审查报告

## 审查日期
2026-05-21

## 审查范围
V1.0-original 核心基础模块（用户管理、登录注册、数据持久化）

---

## 发现的问题与修改

### 1. [安全] 密码明文存储
**问题**: 用户密码以明文形式存储在 `data/users.dat` 文件中，任何有文件访问权限的人都可以直接查看所有用户的密码。

**修改**: 在 `user.cpp` 中增加了 XOR 编码/解码函数 `encode()` 和 `decode()`。密码在存储前经过 XOR 0x5A 编码，读取时解码后比对。构造函数和 `setPassword()` 自动对密码进行编码。

**影响文件**: `user.h`, `user.cpp`

### 2. [稳定性] User::deserialize 缺少格式校验
**问题**: `deserialize()` 直接使用 `std::getline` 读取字段后创建对象，没有校验数据格式。如果数据文件被手动修改或损坏，可能导致创建出字段为空的对象，后续使用时产生未定义行为。

**修改**: 
- 增加用户名和角色的非空检查，为空则返回 NULL
- 学生角色增加专业、班级、学号的非空检查
- `DataManager::loadAll()` 增加对 NULL 返回值的判断

**影响文件**: `user.cpp`, `datamanager.cpp`

### 3. [健壮性] readLine 缺少 EOF 检测
**问题**: `readLine()` 函数没有检测 EOF（如 Ctrl+Z 或管道输入结束），在自动化测试或意外输入结束时会导致死循环或未定义行为。

**修改**: 在 `readLine()` 中增加了 `std::cin.eof()` 检测，与 `readInt()` 保持一致，检测到 EOF 时优雅退出。

**影响文件**: `menu.cpp`

### 4. [功能缺失] 学生无法查看个人信息
**问题**: `Student::displayInfo()` 方法已定义但从未在任何菜单中被调用，学生登录后无法查看自己的注册信息。

**修改**: 
- 学生菜单增加选项4"查看个人信息"
- 实现 `handleStudentInfo()` 方法，调用 `displayInfo()` 展示学生的用户名、专业、班级、学号

**影响文件**: `menu.h`, `menu.cpp`

### 5. [功能缺失] 用户无法修改密码
**问题**: 用户（包括管理员和学生）登录后没有修改密码的功能，初始管理员密码无法更改存在安全隐患。

**修改**: 
- 在 `User` 类中增加 `setPassword()` 方法
- 实现通用的 `handleChangePassword()` 方法，要求验证旧密码后设置新密码
- 学生菜单增加选项5"修改密码"
- 管理员菜单增加选项3"修改密码"
- 密码修改后调用 `DataManager::saveAll()` 持久化

**影响文件**: `user.h`, `user.cpp`, `menu.h`, `menu.cpp`

## 修改文件清单

| 文件 | 修改类型 | 说明 |
|------|----------|------|
| user.h | 修改 | 增加 encode/decode/setPassword 声明，移除 getPassword |
| user.cpp | 修改 | 实现密码编码，deserialize 增加格式校验 |
| datamanager.cpp | 修改 | loadAll 增加 NULL 检查 |
| menu.h | 修改 | 增加 handleStudentInfo/handleChangePassword 声明 |
| menu.cpp | 修改 | 增加 EOF 检测、个人信息查看、密码修改功能 |

## 未解决问题（留待后续版本）

- 密码使用简单 XOR 编码，非真正加密，后续可升级为 SHA256 哈希
- 暂无数据备份和恢复机制
- 管理员无法管理专业和班级列表（需手动编辑数据文件）
