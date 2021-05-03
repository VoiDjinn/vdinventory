#include "register_types.h"
#include "core/class_db.h"
#include "VDInventory.h"

void register_vdinventory_types(){
	ClassDB::register_class<VDIVcInventory>();
    ClassDB::register_class<VDIVcInventoryStock>();
}

void unregister_vdinventory_types() {}
