#ifndef	__PROTOCOL_SEND_H_
#define	__PROTOCOL_SEND_H_

extern void pelcod_call_pre_packet_send(u8 val);
extern void pelcod_set_pre_packet_send(u8 val);
extern void pelcod_open_close_packet_send(u8 val);
extern void pelcod_stop_packet_send(void);
extern void pelcod_lrud_pre_packet_send(u8 lrudcmd,u8 lrspeed,u8 udspeed);
extern void pelcod_zf_packet_send(u8 cmd,u8 zfspeed);


/*****************************************************派尔高协议部分***********************************************/
////////////////各协议同步字/////////////////////////
#define PELCOD_SYNC_WORD 0xff
#define PELCOP_SYNC_WORD 0xa0
#define PELCOP_ETX_WORD 0xaf
/////////////PELCO协议的各种动作命令/////////////////
#define DP_Left  0x04		 //左转
#define DP_Right 0x02		 //右转
#define DP_Up    0x08		 //上转
#define DP_Down  0x10		 //下转
#define DP_ZoomWide   0x40	 //变倍短/广角/缩小/拉远
#define DP_ZoomTele   0x20	 //变倍长/窄角/放大/拉近
#define DP_Left_Wide  0x44   //左缩小
#define DP_Left_Tele  0x24   //左放大
#define DP_Right_Wide  0x42   //右缩小
#define DP_Right_Tele  0x22   //右放大
#define DP_Up_Wide     0x48   //上缩小
#define DP_Up_Tele     0x28   //上放大
#define DP_Down_Wide   0x50   //下缩小
#define DP_Down_Tele   0x30   //下放大

#define DP_Left_Up    0x0c      //左上
#define DP_Left_Down  0x14      //左下
#define DP_Right_Up   0x0a      //右上
#define DP_Right_Down 0x12      //右下

#define DP_Left_Up_Wide  0x4c   //左上缩小
#define DP_Left_Up_Tele  0x2c   //左上放大
#define DP_Right_Up_Wide  0x4a   //右上缩小
#define DP_Right_Up_Tele  0x2a   //右上放大

#define DP_Left_Down_Wide  0x54   //左下缩小
#define DP_Left_Down_Tele  0x34   //左下放大
#define DP_Right_Down_Wide  0x52   //右下缩小
#define DP_Right_Down_Tele  0x32   //右下放大

#define DP_Preassign_Place_Set   0x03   //预置位设置
#define DP_Preassign_Place_Call	 0x07   //预置位调用
#define DP_Preassign_Place_Del   0x05 	//预置位删除

#define DP_Pattern_Start         0x1f   //自学习开始
#define DP_Pattern_Stop          0x21   //自学习停止
#define DP_Pattern_Run           0x23   //自学习运行

#define DP_Tour_Run            0x65   //巡航运行
#define DP_Tour_Stop           0x71   //巡航停止
#define DP_Tour_Del            0x6d   //删除巡航
#define DP_Tour_SetStart       0x67   //开始设置巡航，同时设ID
#define DP_Tour_Data           0x69  //设置巡航数据
#define DP_Tour_SetEnd         0x6b   //设置巡航结束

#define DP_SetZone_Start       0x11   //水平扫描限位开始设置
#define DP_SetZone_End         0x13   //水平扫描限位设置结束
#define DP_ZoneScan_On         0x1b   //开水平扫描
#define DP_ZoneScan_Off        0x1d   //关水平扫描

#define DP_AUX_On              0x09   //设置辅助开关
#define DP_AUX_Off             0x0B   //清除辅助开关

#define D_Auto_Scan              0x90   //自动扫描
#define P_Auto_Scan              0x20   //自动扫描

#define D_Camera_On              0x88   //相机开
#define D_Camera_Off             0x08   //相机关

#define P_Camera_On              0x50   //相机开
#define P_Camera_Off             0x10   //相机关




//////////PELCO-D协议中自有动作命令//////////
#define D_FocusFar   0x80    //聚焦远
#define D_IrZoomout  0x04	 //光圈缩小
#define D_IrZoomIn   0x02	 //光圈扩大
#define D_FocusNear  0x01    //聚焦近

//////////PELCO-P协议中自有动作命令//////////
#define P_IrZoomout    0x08	   //光圈缩小
#define P_IrZoomIn     0x04	   //光圈扩大
#define P_FocusFar     0x01    //聚焦远
#define P_FocusNear    0x02    //聚焦近
#define PRORO_INVILID  0x00    //无效值



#endif
