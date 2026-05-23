# V1.2 代码审查报告

## 审查日期
2026-05-21

## 审查范围
V1.2-original 节目报名模块（学生编辑节目信息、Registration 数据模型）

---

## 发现的问题与修改

### 1. [代码清理] createRegistration 方法冗余
**问题**: `DataManager::createRegistration()` 方法在头文件和实现中定义，但从未被任何调用者使用。学生创建报名通过直接构造 Registration 对象并调用 `saveRegistration()` 完成。

**修改**: 从 `datamanager.h` 和 `datamanager.cpp` 中移除未使用的 `createRegistration()` 方法。

**影响文件**: `datamanager.h`, `datamanager.cpp`

### 2. [功能缺陷] 修改报名后审核状态未重置
**问题**: 当学生的报名被管理员驳回后，学生修改报名信息重新提交时，`saveRegistration()` 只更新了节目信息字段（类型、时长、名称、描述、表演者），但没有重置审核状态。导致修改后的报名仍显示"未通过"状态，管理员需要手动查找才能发现已修改的报名。

**修改**: 在 `saveRegistration()` 的更新路径中，增加了 `setStatus("pending")` 和 `setReviewComment("")` 调用，确保学生每次修改报名后状态自动重置为"待审核"，审核意见清空。

**影响文件**: `datamanager.cpp`

## 修改文件清单

| 文件 | 修改类型 | 说明 |
|------|----------|------|
| datamanager.h | 修改 | 移除未使用的 createRegistration 声明 |
| datamanager.cpp | 修改 | 移除 createRegistration 实现，更新路径增加状态重置 |

## 已确认的设计决策

- 报名数据使用 `|` 分隔符进行文件存储（与用户、公告文件一致）
- 节目类型通过 `Registration::getProgramTypes()` 静态方法获取预设列表
- 审核通过的报名不可修改（UI 层控制）
- 驳回的报名可以修改并重新提交
