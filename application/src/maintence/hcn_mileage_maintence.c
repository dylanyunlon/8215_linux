/**
*
* @file hcn_mileage_maintence.c
*
* @brief This message displayed in Doxygen Files index
*
* @ingroup PackageName
* (note: this needs exactly one @defgroup somewhere)
*
* @date	2025/10/07 14:23
* @author och
*
*/



#ifdef HCN_MILEAGE_MAINTENCE_ENABLE

#define FIRST_MAINTENCE_MILEAGE (1000)      ///< 首保距离
#define NEXT_MAINTENCE_MILEAGE (3000)      ///< 其余保养距离

typedef struct {
    uint16_t count;         ///< 保养次数
    int main_mileage;      ///< 当前段保养距离
    uint32_t temp;
    uint32_t last_main_mileage;  ///< 上次保养保养时的总里程
    int remain_mileage;   ///< 剩余保养里程
    int travel_mileage;  ///< 保养里程范围内已行驶的距离
} mileage_maintenance_t;

static int last_mileage = 0;
static mileage_maintenance_t maintenance; 

void clean_maintenance_state(void) {
   
}

void update_maintence_mileage(int mileage) {
 
}



#endif