#include "edge.h"
#include "cJSON.h"

static void edge_normal_msg_handler_user(char *topic, char *payload)
{
    log_write(LOG_INFO, "topic:%s payload:%s", topic, payload);
    return;
}

static void edge_topo_notify_handler_user(topo_operation opera, char *payload)
{
    log_write(LOG_INFO, "topo change:%d payload:%s", opera, payload);
    return;
}

static void edge_subdev_status_handler_user(subdev_able opera,char *payload)
{
    log_write(LOG_INFO, "subdev:%d payload:%s", opera, payload);
    return;
}

int main(int argc, char **argv)
{
    edge_status         status     = EDGE_OK;
    subdev_client *     subdevClient = NULL;
    cJSON *dirver_info_cfg = NULL;
    cJSON *device_list_cfg = NULL;
    int upload_period = 5;
    char *topic_str = NULL;
    struct timeval stamp;
    char *time_stamp = malloc(100);
    
    status = edge_common_init();
    if(EDGE_OK != status)
    {
        log_write(LOG_ERROR, "edge_Common_Init fail");
        return EDGE_ERR;
    }

    // 设置日志等级为INFO，每个日志大小5M，可以保存6个文件
    status = edge_set_log(LOG_INFO, 5, 6);
    if(EDGE_OK != status)
    {
        log_write(LOG_ERROR, "set log fail");
        return EDGE_ERR;
    }

    edge_set_topo_notify_handle(edge_topo_notify_handler_user);
    
    edge_set_subdev_status_handle(edge_subdev_status_handler_user);

    // 解析驱动配置
    /* 
    {
        "period": 10,
        "topic_config": {
            "auth": "pub",
            "topic": "/%s/%s/upload"
        }
    }
    */
    dirver_info_cfg = cJSON_Parse(edge_get_driver_info());
    if (!dirver_info_cfg) 
    {
        log_write(LOG_ERROR, "edge_get_driver_info parse error: [%s]",cJSON_GetErrorPtr());
        return EDGE_ERR;
    }

    cJSON *period = cJSON_GetObjectItem(dirver_info_cfg, "period");
    if(NULL != period)
    {
        upload_period = period->valueint;
    }
    
    // 解析配置取出topic_config下的topic
    cJSON *config_item = cJSON_GetObjectItem(dirver_info_cfg, "topic_config");
    cJSON *topic_format_json = cJSON_GetObjectItem(config_item, "topic");    

    // 解析设备列表
    device_list_cfg = cJSON_Parse(edge_get_device_info());    
    if (!device_list_cfg) 
    {
        log_write(LOG_ERROR, "edge_get_device_list parse error: [%s]",cJSON_GetErrorPtr());
        return EDGE_ERR;
    }

    // 判断是否绑定子设备，并获取设备列表的productsn和devicesn，将设备上线
    if(cJSON_GetArraySize(device_list_cfg) > 0)
    {
        cJSON *arrary_item = cJSON_GetArrayItem(device_list_cfg, 0);
        cJSON *arrary_productsn = cJSON_GetObjectItem(arrary_item, "productSN");
        cJSON *arrary_devicesn = cJSON_GetObjectItem(arrary_item, "deviceSN");  
        // 组成发送消息topic
        topic_str = (char *)malloc(100);
        if(NULL == topic_str)
            return EDGE_ERR;
        memset(topic_str, 0, 100);
        if(NULL != topic_format_json)
        {
            snprintf(topic_str, 100, topic_format_json->valuestring, arrary_productsn->valuestring, arrary_devicesn->valuestring);
        }
        else
        {
            snprintf(topic_str, 100, "/%s/%s/upload", arrary_productsn->valuestring, arrary_devicesn->valuestring);
        }
        
        // 初始化一个子设备
        subdevClient = edge_subdev_construct(arrary_productsn->valuestring, arrary_devicesn->valuestring, edge_normal_msg_handler_user);
        if(NULL == subdevClient)
        {
            log_write(LOG_ERROR, "edge construct fail!");
            return EDGE_ERR;
        }

        #if 0
        // 动态注册一个子设备
        status = edge_subdev_dynamic_auth(subdevClient, "product_secret", 5000);
        if(EDGE_OK != status)
        {
            printf("edge dynamic auth fail!");
            return EDGE_ERR;
        }
        #endif
        
        // 绑定子设备拓扑
        status = edge_add_topo(subdevClient, 5000);
        if(EDGE_OK != status)
        {
            log_write(LOG_ERROR, "edge_add_topo fail");
            return EDGE_ERR;
        }

        // 查看拓扑
        char *topoa = edge_get_topo(5000);
        if(NULL != topoa)
        {
            log_write(LOG_INFO, "edge_get_topo:%s", topoa);
        }

        // 子设备登录
        status = edge_subdev_login_async(subdevClient);
        if(EDGE_OK != status)
        {
            log_write(LOG_ERROR, "edge_subdev_login fail");
            return EDGE_ERR;
        }
    } 
    
    while(1)
    {            
        // 维持心跳,并周期发送消息
        sleep(upload_period);
        //sleep(1000);

        gettimeofday(&stamp, NULL);
        memset(time_stamp, 0, 100);
        snprintf(time_stamp, 100, "{\"timestamp\": \"%ld\"}", stamp.tv_sec);
        log_write(LOG_DEBUG, "send message[%s]", time_stamp);
        
        status = edge_publish(topic_str, time_stamp);
        if(EDGE_OK != status)
        {
            log_write(LOG_ERROR, "edge_publish fail");
            return EDGE_ERR;
        }
    }
}

