# 软件05-单元测试用例
**文档编号：SW-STD-05**
**版本：V1.0**
**发布日期：2025-11-11**
**编制人：粟工**

# test_network_layer.c

###     void test_network_recv_frame_PCI_SF(void)
- ##### 测试uds_get_frame各个入口参数的不同组合不会发生阻塞、程序崩溃、越界访问

###     void test_network_recv_frame_dlc_0(void)
- ##### 测试network_recv_frame在frame_dlc=0的各个入口参数组合下不会发生阻塞、程序崩溃、越界访问

###     void test_network_recv_frame_dlc_255(void)
- ##### 测试network_recv_frame在frame_dlc=255的各个入口参数组合下不会发生阻塞、程序崩溃、越界访问

---

# test_obd_dtc

###     test_obd_dtc_ctrl_true
- ##### 测试obd_dtc_ctrl(true)能否返回预期值并检查会不会发生阻塞、程序崩溃、越界访问

###     test_obd_dtc_ctrl_false
- ##### 测试obd_dtc_ctrl(false)能否返回预期值并检查会不会发生阻塞、程序崩溃、越界访问

###     test_uds_update_obddtc_NORESULT
- ##### 测试uds_update_obddtc在obd_dtc_test_t test_result参数为NORESULT时的各个入口参数的不同组合不会发生阻塞、程序崩溃、越界访问

###     test_uds_update_obddtc_PASSED
- ##### 测试uds_update_obddtc在obd_dtc_test_t test_result参数为PASSED时的各个入口参数的不同组合不会发生阻塞、程序崩溃、越界访问

###     test_uds_update_obddtc_FAILED
- ##### 测试uds_update_obddtc在obd_dtc_test_t test_result参数为FAILED时的各个入口参数的不同组合不会发生阻塞、程序崩溃、越界访问

###     test_uds_load_obddtc
- ##### 测试get_uds_load_obddtc不会发生阻塞、程序崩溃、越界访问

###     test_uds_save_obddtc
- ##### 测试get_uds_save_obddtc不会发生阻塞、程序崩溃、越界访问

###     test_get_dtc_number_by_status_mask
- ##### 测试get_dtc_number_by_status_mask各个入口参数的不同组合不会发生阻塞、程序崩溃、越界访问并且能够返回预期的结果

###     test_get_dtc_by_status_mask
- ##### 测试get_dtc_by_status_mask各个入口参数的不同组合不会发生阻塞、程序崩溃、越界访问

###     test_get_supported_dtc
- ##### 测试get_supported_dtc各个入口参数的不同组合不会发生阻塞、程序崩溃、越界访问

###     test_clear_dtc_by_group
- ##### 测试clear_dtc_by_group各个入口参数的不同组合不会发生阻塞、程序崩溃、越界访问

## test_get_snapshot_member
####

###     test_get_dtc_extended_data
- ##### 测试get_dtc_extended_data各个入口参数的不同组合不会发生阻塞、程序崩溃、越界访问

---

# test_port_input.c
###     test_ZTai_UDS_Send_ao
- ##### 测试ZTai_UDS_Send在各个正常参数下能否返回预期值并检查会不会发生阻塞、程序崩溃、越界访问

###     test_ZTai_UDS_Send_no
- ##### 测试ZTai_UDS_Send在各个非正常参数下能否返回预期值并检查会不会发生阻塞、程序崩溃、越界访问

---

# test_uds_service.c
###     test_uds_get_frame_ao
- ##### 测试uds_get_frame不会发生阻塞、程序崩溃、越界访问

###     test_uds_main_ao
- ##### 测试uds_main不会发生阻塞、程序崩溃、越界访问

###     test_uds_init_ao
- ##### 测试uds_main不会发生阻塞、程序崩溃、越界访问并能返回预期结果

###     test_uds_timer_start_num0
- ##### 测试uds_main在特定参数下不会发生阻塞、程序崩溃、越界访问并能返回预期结果

###     test_uds_timer_start_num1
- ##### 测试uds_main在特定参数下不会发生阻塞、程序崩溃、越界访问并能返回预期结果

###     test_uds_timer_start_num2
- ##### 测试uds_main在特定参数下不会发生阻塞、程序崩溃、越界访问并能返回预期结果

###     test_app_programming_pos_response_ao
- ##### 测试app_programming_pos_response不会发生阻塞、程序崩溃、越界访问

---

# test_uds_status.c

###     test_uds_security_access_ao
- ##### 测试uds_security_access在各个非正常参数下能否返回预期值并检查会不会发生阻塞、程序崩溃、越界访问

###     test_uds_security_access_no
- ##### 测试uds_security_access在各个非正常参数下能否返回预期值并检查会不会发生阻塞、程序崩溃、越界访问

---

# test_uds_support.c

###     test_uds_load_rwdata_ao
- ##### 测试test_uds_load_rwdata在各个非正常参数下会不会发生阻塞、程序崩溃、越界访问

###     test_uds_load_rwdata_no
- ##### 测试test_uds_load_rwdata在各个非正常参数下会不会发生阻塞、程序崩溃、越界访问

---

# test_uds_util.c

###     test_rand_u8_no
- ##### 测试rand_u8不会发生阻塞、程序崩溃、越界访问并能返回预期结果

###     test_host_to_canl_no
- ##### 测试host_to_canl在特定参数下不会发生阻塞、程序崩溃、越界访问并能返回预期结果

###     test_host_to_canl_ao
- ##### 测试host_to_canl在特定参数下不会发生阻塞、程序崩溃、越界访问并能返回预期结果

###     test_host_to_cans_no
- ##### 测试host_to_cans在特定参数下不会发生阻塞、程序崩溃、越界访问并能返回预期结果

###     test_host_to_cans_ao
- ##### 测试host_to_cans在特定参数下不会发生阻塞、程序崩溃、越界访问并能返回预期结果

###     test_can_to_hostl_no
- ##### 测试can_to_hostl在特定参数下不会发生阻塞、程序崩溃、越界访问并能返回预期结果

###     test_can_to_hostl_ao
- ##### 测试can_to_hostl在特定参数下不会发生阻塞、程序崩溃、越界访问并能返回预期结果

---

# test_user_identifier.c

---