# V32_BOOT_uds_frame
-名称：V32 BOOT部分UDS刷写框架
-版本：0.0.1
-作者：黄俊贤
-时间：2025.11.11

## 1.初始化阶段
#### 1. UDS框架初始化extern int uds_init (void)

- **名称**：uds初始化
- **参数**：void
- **返回值**：0 返回成功 其余失败
- **作用**：用户层uds初始化框架，注册处理接收单帧/首帧/连续帧的回调函数

```c
/**
 * uds_init - uds user layer init
 *
 * @void  :
 *
 * returns:
 *     0 - ok
 */
 //用户层框架初始化
 //处理接收四种帧的回调函数注册
 //初始化初始会话为默认标准会话
 //初始化接收帧控制全局结构体变量N_USData
```

```mermaid
graph TD
    A([开始]) -->B
    B[初始化会话模式] -->C[默认标准会话<br/>UDS_SESSION_STD]
    C -->D[回调函数指向]
    D -->E[首帧接收通知回调指针<br/>ffindication]
    D -->F[多帧接收通知回调指针<br/>indication]
   

    E -->H[首帧接收通知回调函数<br/>static void
uds_dataff_indication （uint16_t msg_dlc）]
    F -->I[多帧接收通知回调函数<br/>static void
uds_data_indication （uint8_t msg_buf【】, uint32_t msg_dlc, n_result_t n_result）]
   

    H -->K[注册回调函数<br/>network_reg_usdata（）<br/>初始化接收帧控制全局结构体变量N_USData]
    I -->K 
  

    K -->L([结束])
```

#### 2. 填充身份信息初始化 void fill_identifier_data_init(void)

- **名称**：填充身份信息初始化
- **参数**：void
- **返回值**：：void
- **作用**：把uds各种身份信息填充，写入flash存储

```C
//填充身份信息初始化
/*
防重复：先检查信息是否已存在，存在就不重复初始化；
理数据：把 ECU 的 11 类关键信息（版本、序号、零件号等）整理好，空值填空格，保证格式统一；
存档案：把整理好的信息存到 FLASH 的指定位置，再盖个 “已完成” 的章，方便后续诊断仪用 UDS 服务（比如 0x22 服务）读取这些信息。
```

```mermaid
graph TD
    A([开始]) 
    A -->B{判断身份信息有无初始化过？}
    B -->|已初始化|C[跳过]
    B -->|未初始化|D[进入初始化流程]
    C -->Z

    D -->E[读出旧数据信息]

    E -->F[零件号]
    F -->G[供应商ID]
    F -->H[系统名 ]
    F -->I[LIN 总线软件标识]
    F -->J[ECU硬件版本]
    F -->K[BOOTLOADER引导程序版本]
    F -->L[车辆识别码]
    F -->M[预留指纹标识]
    F -->N[供应商软件版本]
    F -->O[软件版本]
    F -->P[软件ID]

    F -->R[向FLASH写入身份信息]
    G -->R
    H -->R
    I -->R
    J -->R
    K -->R
    L -->R
    M -->R
    N -->R
    O -->R
    P -->R

    R -->T[写入成功]
    T -->U[将写入身份信息成功标志写入标记信息的存储区]

    U -->Z

    Z([结束])

```

#### 3. 处理停留在boot的场合 void process_stay_boot_suitation(void)

- **名称**：处理停留在boot的情况
- **参数**：void
- **返回值**：：void
- **作用**：读取FALSH标志位，查看是否在APP把标志位置为停留BOOT，若置位，则进入编程会话进入后面的刷写准备环节。

```C
//处理停留在boot的场合
//当APP接收到10 02服务，停留标BOOT标志STAY_BOOT_STATUS被置位，复位进入boot，便触发进入处理停留BOOT情况
```

```mermaid
graph TD
    A([开始]) 
    A -->B{判断FLASH里停留标BOOT标志是否置位？}
    B -->|是|C[处理停留BOOT情况]-->C1[停留boot标志变量stay_boot_flag置为1]-->E
    B -->|否|D[跳过] -->Z

    E[FLASH里停留BOOT标志置为无效]
    E -->F[FLASH里下载APP状态标志置为未擦除]

    F -->G[积极响应诊断仪已进入编程会话]

    G -->H[打开S3定时器]

    H -->Z

    Z([结束])


```


## 2.任务调度阶段

### 1ms任务调度

#### 1. uds_main();

- **名称**：uds 运行主函数
- **参数**：void
- **返回值**：：void
- **作用**：用户层uds运行主函数，负责执行S3定时器发生超时，27服务超时，连续帧接收超时处理及连续帧发送间隔时间控制

```c
/**
 * uds_main - uds main loop, should be schedule every 1 ms
 *
 * @void  :
 *
 * returns:
 *     void
 */
```

```mermaid
graph TD
    A([开始]) --> B[网络层调度主函数<br/>network_main （）]
    A --> C{S3 服务定时器UDS_TIMER_S3server是否超时?}
    C --> D[是]
    C --> E[否]
    
    D --> F{判断当前会话状态是?}
    F --> G[拓展会话]
    F --> H[编程会话]
    
    G --> I[会话降为默认会话UDS_SESSION_STD<br/>
    安全等级重置为未解锁UDS_SA_NON（最低权限）<br/>
    停止功能寻址激活定时器UDS_TIMER_FSA <br/>
    安全访问失败计数器uds_fsa_cnt清零<br/>
    APP下载状态改为未知DS_UNKNOW]
    H --> I
    I --> J{功能寻址激活定时器<br/>UDS_TIMER_FSA是否超时?}  
    J --> K[是]
    J --> L[否]
    K --> M[安全访问失败计数器uds_fsa_cnt清零 <br/> 种子请求标记req_seed清零]

    E --> J
    L --> S([结束])
    M --> S


```
#### 2. extern void network_main (void)

- **名称**：uds 网络层控制主函数
- **参数**：void
- **返回值**：：void
- **作用**：负责连续帧接收超时处理及连续帧发送间隔控制

```c
/**
 * network_main - network main task, should be schedule every one ms
 *
 * @void
 *
 * returns:
 *     void
 */
//接收端网络层超时处理
//设计为每 1ms 调度一次，负责处理网络层的定时器超时事件和多帧传输的进度管理。
//它是网络层状态机运行的 “心脏”，确保多帧传输按节奏进行，并在超时等异常情况下进行异常处理
```

```mermaid
graph TD
    A([开始]) 
    A --> B{判断连续帧接收超时定时器TIMER_N_CR是否超时?}
    B --> |是|C[重置网络层状态clear_network（）<br/>（终止当前多帧接收）<br/>  向上层报告N_TIMEOUT_Cr（连续帧接收超时）] 
    B --> |否|D[跳过]
    C --> K
    D --> K{判断连续帧发送间隔超时定时器TIMER_STmin是否超时?}
    K --> |否|Y[跳过]
    K --> |是|L[ 发送当前剩余的连续帧 send_consecutiveframe <br/> 更新剩余未发送位置 <br/> 更新剩余长度]
    L --> M{判断是否有剩余长度没有发完？}
    M --> |否|N[清空并停止定时器 <br/> clear_network（）]
    M --> |是|O{判断块g_rfc_bs大小有没有限制}
    O --> |否|P[开启连续帧发送间隔超时定时器TIMER_STmin计时，等待下一次的超时 <br/>nt_timer_start（TIMER_STmin）]
    O --> |是|Q[已发送连续帧累加g_xcf_bc++]
    Q --> R{判断发送连续帧块计数g_xcf_b是否达到块大小?}
    R -->|是|Z
    R -->|否|T[开启连续帧发送间隔超时定时器TIMER_STmin计时，等待下一次的超时 <br/>nt_timer_start（TIMER_STmin）]
    T -->Z([结束])
    N -->Z
    P -->Z
    Y -->Z

```




### while循环任务调度
#### 3.主任务void main_task(void)

- **名称**：mcu主任务调度
- **参数**：void
- **返回值**：void
- **作用**：负责主任务调度，包括：喂狗任务，mcu复位任务，超时释放uds队列任务，LIN帧出队接收和处理任务，刷写完成进入APP任务等。

```c
//主任务调度
//刷写成功校验完成后进入APP任务
//负责将LIN帧数据进行出队接收和处理
//负责MCU的复位任务
//负责喂狗任务
//负责超时释放uds队列任务
```

```mermaid
graph TD
    A([开始]) 
    A -->B[lin处理任务<br/>lin_task（）]
    A -->C[mcu复位任务<br/>mcu_reset_task（）]
    A -->X1[跳转APP任务<br/>jump_to_app_task（）]
    A -->W1[释放队列任务<br/>release_uds_queue_task（）]
    A -->V1[看门狗任务<br/>feed_wdt_task（）] -->V2[每隔1秒喂狗一次]-->Z1

    X1 -->X2{判断stay_boot_flag是否为1？}
    X2 -->|是|X3[跳出]-->Z1
    X2 -->|否|X4[只执行一次判断操作]
    X4 -->X5[关闭看门狗]
    X5 -->X6{判断APP下载情况是否为未知或者已校验}
    X6 -->|是|X7[跳转至APP]
    X6 -->|否|X8[不跳转APP]   
    X7 -->Z1
    X8 -->Z1

    W1 -->W2{判断uds队列不为空且定时器计数是否大于5秒？}
    W2 -->|是|W3[清空uds队列] -->Z1
    W2 -->|否|W4[跳过] -->Z1


    B -->D[静态初始化lin接收状态lin_rx_st为初始状态LIN_INIT_STATUS]
    D -->E{若lin_rx_st不为态初始状态LIN_INIT_STATUS 且 S3服务定时器发生超时【5s】}
    E -->|是|G[清空lin队列和uds队列]
    E -->|否|H[跳过]
    
    H -->F{while判断循环lin队列出队}
    F -->|有数据|I[出队]
    F -->|无数据|J[跳过]

    I -->K{根据ID判断此LIN帧是 <br/> 接收帧SLAVE_RECEIVE还是响应帧SLAVE_RESPONSE}
    K -->|接收帧|S[调用ID为3C的接收帧处理函数process_diag_receive（）]
    K -->|响应帧|T[调用ID为3D的响应帧处理函数process_diag_response（）]


    
    S -->U{判断接收帧的首字节是否为0x68，检验NAD是否正确}
    T -->V[处理响应帧]

    U -->|是|W[处理接收帧]
    U -->|否|X[不处理]

    C -->Y{获取并判断复位标志mcu_reset_flag是否被置1}

    Y -->|是|Y1[等待20ms后 <br/> mcu_reset_flag置0 <br/> prepare_reset_flag置1]
    Y -->|否|Y2[跳过]

    Y1 -->Y3{判断准备复位标志prepare_reset_flag是否被置1}
    Y3 -->|是|Y4[1.写flash记录停留BOOT有效标志位 <br/> 2.执行软件复位 ]
    Y3 -->|否|Y5[跳过]


    Z1([结束])

    Y2 -->Z1
    Y4 -->Z1
    Y5 -->Z1
    G -->Z1
    V -->Z1
    X  -->Z1
    W  -->Z1
    J -->Z1

```
#### 4.处理接收帧 
#### 1.extern void network_recv_frame (uint8_t func_addr, uint8_t frame_buf[], uint8_t frame_dlc)

- **名称**：处理网络层uds接收帧
- **参数**：
func_addr ：寻址方式
frame_buf ：数据帧缓冲区
frame_dlc ：数据帧长度
- **返回值**：：void
- **作用**：将接收的数据帧进行解析分类处理，对应分成：连续帧，单帧，首帧，进行对应的处理


```C
//网络层通过network_recv_frame函数“接收”底层总线（如 CAN）传来的帧，按 PCI（协议控制信息）帧类型分流处理：
/**
 * network_recv_frame - recieved uds network can frame
 *
 * @func_addr : 0 - physical addr, 1 - functional addr  //寻址类型（0 = 物理寻址，1 = 功能寻址）；
 * @frame_buf : uds can frame data buffer               //接收的帧数据缓冲区（总线传来的原始数据）；
 * @frame_dlc : uds can frame length                    //帧数据长度（DLC，Data Length Code）。
 *
 * returns:
 *     void
 */
```

```mermaid
graph TD
    A([开始]) 
    A -->B{判断数据帧首字节是否为正确的NAD？}
    B -->|是|C[解析LIN数据]
    B -->|否|Z

    C -->D[从首字节解析PCI类型]
    D -->E[单帧]
    D -->F[首帧]
    D -->G[连续帧]


    E -->J{判断是否为0x3E服务帧且此时正在多帧接收？}
    J -->|是|K[告知上层收到未预期的PDU]
    J -->|否|L[跳过]

    K -->M[接收单帧recv_singleframe （frame_buf, frame_dlc）]
    L -->M

    M -->N{判断是否为 3E 80会话}
    N -->|是|O[打开S3定时器保持会话]
    N -->|普通单帧|P[通知上层处理有效单帧数据 <br/> uds_data_indication（）]
    O -->P

    F -->Q{判断当前接收状态nwl_st是否为接收连续帧状态NWL_RECV？}
    Q -->|是|R[告知上层收到未预期的 PDU]
    Q -->|否|S[跳过]

    R -->T[接收首帧recv_firstframe（frame_buf, frame_dlc）<br/> 当前接收状态nwl_st 置为接收连续帧状态 NWL_RECV ]
    S -->T

    T -->U[打开连续帧接收超时定时器TIMER_N_CR <br/>  等待连续帧标志g_wait_cf 置为 TRUE]
    U -->V[通知上层处理有效首帧uds_dataff_indication（）]

    G -->W{判断是否当前接收状态nwl_st 为 NWL_RECV <br/> 且 等待连续帧标志 g_wait_cf 为TRUE}

    W -->|是|X[接收连续帧recv_consecutiveframe （uint8_t frame_buf【】, uint8_t frame_dlc）]
    W -->|否|Y[跳过]

    X -->Y1{判断连续帧是否接收完毕？}
    Y1 -->|是|Y2[通知上层处理有效连续帧数据 <br/> uds_data_indication（）]
    Y1 -->|否|Y3[重启连续帧接收超时定时器TIMER_N_CR <br/> 继续接收]

    Y -->Z
    Y2-->Z
    Y3-->Z
    V -->Z
    P -->Z

    Z([结束])

```

#### 2.数据请求调用服务列表
#### static void  uds_data_indication (uint8_t msg_buf[], uint32_t msg_dlc, n_result_t n_result)

- **名称**：uds数据请求调用服务列表
- **参数**：
msg_buf ：数据帧缓冲区
msg_dlc ：数据帧长度
n_result：结果信息
- **返回值**：：void
- **作用**：将接收的lin数据解析进行长度，服务列表查询等各种校验，合格的数据最后在服务列表搜寻对应的服务函数地址，进入对应的服务处理


```C
/**
 * uds_data_indication - uds data request indication callback, 
 *
 * @msg_buf  :
 * @msg_dlc  :
 * @n_result :
 *
 * returns:
 *     void
 */
//根据接收过来正确的的多帧[首帧+连续帧]，进行信息校验，最后根据信息位调用对应的服务请求
//UDS（统一诊断服务）模块的核心入口函数——uds_data_indication，它是底层网络（如 CAN/LIN）接收诊断请求后的 “回调处理中心”。
//其核心作用是对收到的诊断报文进行多维度合法性校验，确保只有符合条件的请求才会被分发到对应的 UDS 服务函数（如uds_service_10、uds_service_22）处理
```

```mermaid

graph TD
    A([开始]) 
    A -->B[暂停S3服务器定时器  <br/> 提取服务ID  <br/> 提取是否抑制正响应标志]
    B -->C[遍历服务列表]
    C -->D{判断当前请求服务是否在服务列表之内？}
    D -->|是|E{当前若是功能寻址且服务不支持功能寻址？}
    D -->|否|F[跳过]
    E -->|是|G[跳过]
    E -->|否|H{报文长度校验是否合规？}
    H -->|否|I[返回长度不合规否定响应]
    H -->|是|J{判断当前安全等级权限是否大于该服务需要的最低安全等级}
    J -->|符合|K[调用对应服务的处理函数]
    J -->|不符合|L[权限不足：返回安全访问拒绝否定响应]
    K -->M[进入各个诊断服务]
    M -->N[处理服务请求]
    N -->|肯定响应|O[肯定响应处理]
    N -->|否定响应|P[否定响应处理]

    O -->Q[打开S3服务定时器]
    Q -->R[发送UDS信息]
    R -->|发送单帧|S[UDS报文存放入uds队列]
    R -->|发送多帧|T{判断是否发送首帧？}
    

    U[外部连续帧最小间隔定时器TIMER_STmin发生超时] -->V[发送连续帧] 
  
    P -->Q
    T -->|发完首帧|U
    V -->S
    T -->|未发首帧|Y1
    
    Y1[准备首帧数据 <br/> 打开发送连续帧最小间隔定时器TIMER_STmin]

    Y1 -->Y2[发送首帧]
    Y2 -->S

    S -->Z
    F-->Z
    G-->Z
    I-->Z
    L-->Z
    
    Z([结束])


```


#### 5.处理响应帧
- **作用**：等待3D的帧头，将uds队列存放处理好的响应帧数据，进行出队发送给主机响应

```C
//uds队列存放处理完的响应帧，等待主机3D帧头发来即可响应发送LIN数据
//若接收帧需要正响应且NAD正确和队列中无数据，发送忙响应
```

```mermaid
graph TD
    A([开始]) 
    A -->B{判断uds队列是否有数据？}
    B -->|有数据|C[从uds队列获取数据]
    B -->|无数据|D{判断是否满足发送的3C接收帧需要正响应且NAD正确？}

    D -->|是|E[发送服务忙响应]
    D -->|否|F[跳过]

    C -->G[发送处理完的LIN数据给主机]

    E -->Z
    G -->Z
    F -->Z

    Z([结束])
```