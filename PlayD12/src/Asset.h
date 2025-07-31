#pragma once

#include "PCH.h"  

#include "Base.h"
#include "Render/Loader.h"
#include "Render/Texture.h"

inline SharedPtr<FD3D12Texture> LoadTexture(std::string_view path, Loader::TextureImportConfig config = {});


struct AssetManager {
	static AssetManager& Get();

	void LoadResources();

	std::unordered_map<std::string, Loader::D3D12ImageData> textures;
};


// Asset identity (content addressable or registry-issued)
using AssetID = uint64_t;

enum class AssetType { Texture2D, TextureCube, Mesh, Material, Shader, Unknown };

// Uniform URI scheme: file://, engine://, proc://, http(s):// (future)
struct AssetURI {
    std::string scheme; // "engine", "file", "proc"
    std::string path;   // "textures/brick_wall.ktx2" or "sphere"
    std::string query;  // "radius=1&subdiv=64"
};

// Strong/weak handle to a typed asset
template<typename T>
class AssetHandle {
public:
    AssetHandle() = default;
    explicit AssetHandle(AssetID id) : m_id(id) {}
    bool IsReady() const noexcept;
    T* Get() const noexcept;        // nullptr if not ready
    AssetID Id() const noexcept { return m_id; }

    // Install a callback when ready (for async)
    void OnReady(std::function<void(T&)> cb) const;

private:
    AssetID m_id{ 0 };
};

enum class AssetState : uint8_t {
    Unloaded,      // 初始态
    LoadingCPU,    // 读盘/I/O/解码中
    LoadedCPU,     // CPU 数据已就绪（像素/顶点/索引/元数据）
    UploadingGPU,  // 正在拷贝到显存
    Ready,         // 可供渲染使用（SRV/RTV 等已准备好）
    EvictedGPU,    // 显存回收（可重上传）
    EvictedCPU     // CPU 数据也回收（需重新 I/O）
};


struct RawBlob { std::vector<uint8_t> bytes; };

struct CookedTexture {
    // runtime-friendly payload: precompressed blocks, mips, array layers...
    DXGI_FORMAT format;
    uint32_t width, height, mipLevels;
    std::vector<MipDesc> mips; // file offsets or memory slices
};

struct CookedMesh {
    // tightly packed CPU blobs ready for GPU copy; or already GPU-friendly layout
    BufferDesc vertex, index;
    VertexDecl decl;
    BoundingBox bbox;
    // optional: LODs, meshlets, cluster data...
};

class IImporter {
public:
    virtual bool CanImport(const AssetURI&) const = 0;
    virtual RawBlob Import(const AssetURI&) = 0; // pull raw file/proc data
};

class ICooker {
public:
    virtual bool CanCook(AssetType type, const AssetURI&) const = 0;
    virtual std::variant<CookedTexture, CookedMesh> Cook(const RawBlob&, const AssetURI&) = 0;
};

class IResourceLoader {
public:
    virtual AssetType Type() const = 0;
    virtual void LoadCPU(AssetID, const AssetURI&);    // schedules Import+Cook
    virtual void CreateGPU(AssetID, const CookedTexture& or CookedMesh&); // Enqueue upload
};


enum class PrimitiveKind { Sphere, Box, Cylinder, Torus, Grid, ... };

struct SphereParams {
    float radius = 1.f;
    uint32_t subdiv = 64;   // 细分段数（经/纬或正二十面体细分）
    bool    geodesic = false;
    // UV layout, seam policy, antipodal fix, etc.
};

struct ParamMeshKey {
    PrimitiveKind kind;
    std::string   paramJson;  // 规范化序列化后的 JSON；或结构体哈希
};

// canonical hashing (content-addressed)
uint64_t HashParamMeshKey(const ParamMeshKey&);


class IParamMeshGenerator {
public:
    virtual bool CanGen(const ParamMeshKey&) const = 0;
    virtual CookedMesh Generate(const ParamMeshKey&) = 0; // CPU 生成 or GPU compute 生成
};


// textures
struct TextureDesc {
    uint32_t width = 0, height = 0, mipLevels = 1;
    DXGI_FORMAT format = DXGI_FORMAT_BC7_UNORM; // example
    // flags: SRV/RTV/UAV, streamingAllowed, etc.
};

class Texture2D { /* RHI wrapper: resource + SRV + state tracking */ };

class TextureLoader : public IResourceLoader {
public:
    AssetType Type() const override { return AssetType::Texture2D; }
    void LoadCPU(AssetID, const AssetURI&) override;        // schedule import+cook
    void CreateGPU(AssetID, const CookedTexture&) override; // enqueue upload
};

// meshes
struct MeshDesc {
    VertexDecl decl;
    uint32_t vertexCount, indexCount;
    // per-submesh, material slots, etc.
};
class Mesh { /* GPU buffers + views + bounds */ };

class MeshLoader : public IResourceLoader {
    // similar pattern
};





FApp::GetDeltaTime() / FApp::SetFixedDeltaTime()

塞尔达的
时停
加速
回溯


asset system;



Q:
UE Niagara中来自“system”的参数主要有哪些
是指System namespace，
System层面管理，向下提供的参数，

.Age
Niagara System实例已运行的时间（秒

.LoopCount
如果是looping模式

.Position
.Rotation
.Scale
world space;

.LocalToWorld

.UserXXX

.NumEmitters

.EmitterIndex

.EngineTime

.DeltaTime


Q：
通过Niagara这种系统为例，你能总结业界对于设计一个“system”，有哪些经验，best practice，设计哲学等等


A :

System 不是某个单一功能的集合，
而是上层组织单元
用来协调、驱动、统一管理模块生命周期、
数据流, 配置

keywords :
数据流decoupling
extentible
configurable
coordinating
参数scoping
lifecycle scheduling


分工上，
下层的Emitter负责“做 / imperiative”，
system的未来趋势是 更多追求declarative, data - driven,
open to toolchain;

向下expose内部全局状态，传
下层Node 引用System ；

所谓system - level，
应有高内聚，低耦合，
一定独立地扩展和演化，


工程可扩展，美术可控，
支持“注入 / 绑定” 参数 / 依赖 ，

通过外部数据接口多种方式，和外部交互
level, blueprint, cpp，script, .
user params，



namespace分层设计
提升可读，可维护性；


== declarative声明式 vs imperative命令式

命令式be like :
do A,
execute B,
trigger C,
典型的Cpp, OOP logic, 面向过程，


声明：
what it is，if A, then B,
描述状态，关系，

而不在于如何实现；
不手动维护行为和顺序；

传统场景：
SQL query,
React UI desc,
shader node system,


Niagara的参数绑定 / 数据流中体现于，
本身就是数据流node graph驱动，

描述 模块，节点间的依赖，数据流向，

不直接关心 数据执行顺序，更新，帧同步，

在于上层


声明式系统

模块 / 参数的动态性，复用性，
高度数据驱动 ，

解耦 逻辑 vs 实现，
专注于表达意图，

一个重要的工程视角是，
问题在于想要解决的问题本身是不是 可以设计为数据驱动的；

所谓易于 可视化和工具化，
或者说可视化工具 天然适合用声明式系统去实现，

如果我希望面向artist做一个graph tool，
那大概率底层最好有一个直接的结构描述，
为什么modern engine必须有metadata / reflection / type system,

如果我已经有了完全基于声明的系统，
那映射 / 可视化， 也就实现了tooling;





//a great prompt

with all that being said,
我需要你帮我起草一个有一定严肃性的“system”这边的管理层，
首先是time,
现在能想到我会需要他，
提供physics的以固定步长update的调度，独立于main loop;
提供cpu side的profiling基础，

未来我希望提供基于debug输入，
暂停，
倍速，
推进帧framebugging，
这种需求时也能自然做到；


A：
goals :
多time domains
real / wall
不受影响的 真实时间，
独立于游戏的暂停，倍速，

engine / frame
可scale，可暂停
可变步长variable step

sim / phyics 和渲染解耦 ；
fixed - step
accumualtor追及realtime;


scheduling
基于realtime + scale 得到engine dt


Registration / Scheduling

ITickFixed
ITickFrame
PumpFixedSteps()


Determinism
固定步长 + 明确的更新顺序order是确保可复现实验 / 回放的基础

console / platform msg
->输入系统 -
> 调度，
pause / step 的意图在同一帧生效。



过渡期方案：如果暂时不改各系统接口，你也可以不注册，
直接取 auto dt = g_time.GetTimeInfo().engineDelta;，
用它替换你现在的 0.016f：

m_physicsScene->Tick(fixedDt) 放入 PumpFixedSteps()，
m_renderer->OnUpdate(dt) / m_worldManager->Update(dt) 放到 EndFrame() 或主循环中显式调用。



跑 0..N 次子步，支持最大追赶步数防“死亡螺旋”（spiral of death）
maxCatchupSteps 一定要有；
在低帧率时发生“长时间blocking”地连跑几十个物理步。
如果连续落后，宁可丢弃部分物理步或进行clamp。

如果你希望物理separate thread，
可以把 PumpFixedSteps 移到物理线程，
并用时间切片 + 同步屏障对接渲染
（此处先给单线程设计，便于落地）。


render interpolation
如果后续你想让渲染在物理两个子步间插值，
可以把 m_accumulator / fixedDt 作为插值因子传给渲染层

你已有的 WorldManager / Renderer 分层适合演进
将 PumpFixedSteps() 放到物理线程，
和渲染线程用“三缓冲 + 帧同步栅栏”交换只读变换数据
eg : transforms

FrameTimeInfo 暴露给 UI，即可做debug panel;


以 RAII 为核心的 Profiler 能在不影响架构的前提下逐步升级
//PIX有的轮子我就没必要看



==


接下来我需要你起草有一定严肃性的asset系统；
by which i mean,
我发现Object每次定义时，同时load texture是一个没什么道理的事；
除了其优雅性本身，还有何时上传到gpu，如何复用等技术问题；
为此我觉得大概就是engine那种asset概念吧；

geometry data也同理，
只是我没想好如何去管理，say，都是sphere，但有不同subdivision等参数的这类geometry；


A :
解耦declaration  vs  consumption;
IO统一处理；
CPU decode / cook
GPU upload / residency
ready to use

这个过程是observable states，
潜在的async task,


asset manager
设计AssetHandle<T>
Load<T>
url
id
Pump...

handle.IsReady()


AssetRegistry：
负责将人类可读的 URI / 路径、tags、dependencies
映射到machine可读的稳定，唯一的 ID。

engine ://textures/brick_wall.ktx2
proc://sphere?radius=1&subdiv=64

content - addressed,
hash - based
或规范化paramater - key  canonicalization



程序化 和 file source做统一处理，

以parm as key的方式缓存与复用：
同sphere，
但 subdiv 不同即可视为不同variant。

ResourceLoaders /
importer / cookers
从storage - friendly
cook为runtime - friendly 



有计划地统一upload / recycle, streaming, 考虑memory budget