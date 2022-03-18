#include "VDInventory.h"

VDInventory::VDInventory() {}

void VDInventory::_bind_methods() {
    ClassDB::bind_method ( D_METHOD ( "set_item_stock", "item", "item_id", "amount" ), &VDInventory::set_item_stock );
    ClassDB::bind_method ( D_METHOD ( "remove_item_stock", "item_id" ), &VDInventory::remove_item_stock );
    ClassDB::bind_method ( D_METHOD ( "get_item_stock", "item_id" ), &VDInventory::get_item_stock );
    ClassDB::bind_method ( D_METHOD ( "get_item_stock_amount", "item_id" ), &VDInventory::get_item_stock_amount );
    ClassDB::bind_method ( D_METHOD ( "get_item_stock_object", "item_id" ), &VDInventory::get_item_stock_object );
    ClassDB::bind_method ( D_METHOD ( "transfer_item", "other_inventory", "item_id", "amount", "strict_transfer" ), &VDInventory::transfer_item );

    // ClassDB::bind_method(D_METHOD("get_item_aliases"), &VDInventory::get_item_aliases);
    ClassDB::bind_method ( D_METHOD ( "get_item_stocks" ), &VDInventory::get_item_stocks );
    ClassDB::bind_method ( D_METHOD ( "set_amount_usage_flag", "flag" ), &VDInventory::set_amount_usage_flag );
    ClassDB::bind_method ( D_METHOD ( "get_amount_usage_flag" ), &VDInventory::get_amount_usage_flag );

    BIND_VMETHOD ( MethodInfo ( "stock_creation", PropertyInfo ( Variant::OBJECT, "item" ), PropertyInfo ( Variant::STRING, "alias" ) ) );

    ADD_PROPERTY ( PropertyInfo ( Variant::INT, "amountUsage", PROPERTY_HINT_ENUM, "KEEP_ON_ZERO, KEEP_ON_NEGATIVE, REMOVE_ON_ZERO, REMOVE_ON_NEGATIVE, KEEP_NON_POSITIVE, REMOVE_NON_POSITIVE" ), "set_amount_usage_flag", "get_amount_usage_flag" );

    ADD_SIGNAL ( MethodInfo ( "item_stock_added", PropertyInfo ( Variant::OBJECT, "new_stock_instance", PROPERTY_HINT_RESOURCE_TYPE, "VDInventoryStock" ), PropertyInfo ( Variant::STRING, "new_stock_id" ) ) );
    ADD_SIGNAL ( MethodInfo ( "item_stock_removed", PropertyInfo ( Variant::OBJECT, "old_stock_instance", PROPERTY_HINT_RESOURCE_TYPE, "VDInventoryStock" ), PropertyInfo ( Variant::STRING, "old_stock_id" ) ) );
    ADD_SIGNAL ( MethodInfo ( "item_stock_changed", PropertyInfo ( Variant::OBJECT, "changed_stock_instance", PROPERTY_HINT_RESOURCE_TYPE, "VDInventoryStock" ), PropertyInfo ( Variant::STRING, "stock_id" ), PropertyInfo ( Variant::INT, "new_amount" ), PropertyInfo ( Variant::INT, "old_amount" ) ) );
    ADD_SIGNAL ( MethodInfo ( "item_transferred", PropertyInfo ( Variant::OBJECT, "source_stock" ), PropertyInfo ( Variant::STRING, "stock_id" ), PropertyInfo ( Variant::OBJECT, "target_inventory" ), PropertyInfo ( Variant::OBJECT, "transferred_amount" ) ) );
    ADD_SIGNAL ( MethodInfo ( "transfer_insufficient_amount", PropertyInfo ( Variant::OBJECT, "source_stock" ), PropertyInfo ( Variant::STRING, "stock_id" ), PropertyInfo ( Variant::OBJECT, "target_inventory" ), PropertyInfo ( Variant::INT, "requested_amount" ), PropertyInfo ( Variant::INT, "missing_amount" ) ) );

    BIND_ENUM_CONSTANT ( KEEP_ON_ZERO );
    BIND_ENUM_CONSTANT ( KEEP_ON_NEGATIVE );
    BIND_ENUM_CONSTANT ( REMOVE_ON_ZERO );
    BIND_ENUM_CONSTANT ( REMOVE_ON_NEGATIVE );
    BIND_ENUM_CONSTANT ( KEEP_NON_POSITIVE );
    BIND_ENUM_CONSTANT ( REMOVE_NON_POSITIVE );
}

Ref<VDInventoryStock> VDInventory::create_item_stock ( const Variant &item, String alias ) {
    Ref<VDInventoryStock> new_stock = call ( "stock_creation", item, alias );
    if ( new_stock.is_valid() ) {
        new_stock->owning_inventory = Ref<VDInventory> ( this );
        new_stock->stock_alias = alias;
        new_stock->set_item_object ( item );
    }
    return new_stock;
}

Ref<VDInventoryStock> VDInventory::stock_creation ( const Variant &item, String alias ) {
    Ref<VDInventoryStock> new_stock_item;
    new_stock_item.instance();
    return new_stock_item;
}

Ref<VDInventoryStock> VDInventory::set_item_stock ( const Variant &item, String item_id, int amount ) {
    Ref<VDInventoryStock> stock_item = nullptr;
    if ( item_stocks.has ( item_id ) ) {
        stock_item = item_stocks[item_id];
        int old_amount = stock_item->get_item_amount();
        int amount_dif = old_amount + amount;
        bool change_valid = false;
        if ( amount_dif <= 0 ) {
            AmountUsageFlag usageFlag = this->amountUsage;
            if ( stock_item->customUsage == VDInventoryStock::CustomAmountUsage::FROM_INVENTORY ) {
                usageFlag = stock_item->amountUsage;
            }
            if ( usageFlag == KEEP_ON_ZERO || usageFlag == KEEP_ON_ZERO ) {
                change_valid = amount >= 0;
            } else if ( usageFlag == KEEP_ON_NEGATIVE || usageFlag == REMOVE_ON_ZERO ) {
                change_valid = amount != 0;
            } else if ( usageFlag == KEEP_NON_POSITIVE ) {
                change_valid = true;
            } else if ( usageFlag == KEEP_NON_POSITIVE ) {
                change_valid = amount > 0;
            }
        }
        if ( change_valid ) {
            int stock_item_amount = stock_item->get_item_amount();
            stock_item->set_item_amount ( stock_item_amount += amount );
            emit_signal ( "item_stock_changed", stock_item, item_id, stock_item->get_item_amount(), old_amount );
        } else {
            remove_item_stock ( item_id );
        }
    } else if ( !item_stocks.has ( item_id ) && amount > 0 ) {
        Ref<VDInventoryStock> new_stock = create_item_stock ( item, item_id );
        if ( new_stock.is_valid() ) {
            new_stock->set_item_amount ( amount );
            item_stocks[item_id] = new_stock;
            emit_signal ( "item_stock_added", new_stock, item_id );
            stock_item = new_stock;
        }
    }
    return stock_item;
}

void VDInventory::remove_item_stock ( String item_id ) {
    if ( item_stocks.has ( item_id ) ) {
        Ref<VDInventoryStock> old_stock = item_stocks[item_id];
        item_stocks.erase ( item_id );
        emit_signal ( "item_stock_removed", old_stock, item_id );
    }
}

// void VDInventory::remove_all_stocks(){
// for()
// }

void VDInventory::transfer_item ( Ref<VDInventory> other_inventory, String item_id, int amount, bool strict_transfer ) {
    if ( amount > 0 && item_stocks.has ( item_id ) ) {
        Ref<VDInventoryStock> stock = item_stocks[item_id];
        int amount_dif = stock->get_item_amount() - amount;
        if ( strict_transfer && amount_dif <= 0 ) {
            emit_signal ( "transfer_insufficient_amount", stock, item_id, other_inventory, amount, ( amount_dif ) * ( -1 ) );
        } else {
            // TODO: unfinished - make properly transfer to other inventory, also: Signal for "item_transferred"
            int transferring_amount = amount - amount_dif;
            other_inventory->set_item_stock ( stock->get_item_object(), item_id, transferring_amount );
            set_item_stock ( stock->get_item_object(), item_id, -transferring_amount );
            emit_signal ( "item_transferred", stock, item_id, other_inventory, transferring_amount );
        }
    }
}

Ref<VDInventoryStock> VDInventory::get_item_stock ( String id ) {
    if ( item_stocks.has ( id ) ) {
        Ref<VDInventoryStock> stock = item_stocks[id];
        return stock;
    }
    return nullptr;
}

int VDInventory::get_item_stock_amount ( String item_id ) {
    Ref<VDInventoryStock> stock = this->get_item_stock ( item_id );
    if ( stock.is_valid() ) {
        return stock->get_item_amount();
    }
    return 0;
}

Variant VDInventory::get_item_stock_object ( String item_id ) {
    Ref<VDInventoryStock> stock = this->get_item_stock ( item_id );
    if ( stock.is_valid() ) {
        return stock->get_item_object();
    }
    return Variant();
}

List<String> VDInventory::get_item_aliases() {
    List<String> laliases;
    this->item_stocks.get_key_list ( &laliases );
    return laliases;
}

Array VDInventory::get_item_stocks() {
    const List<String> aliases = this->get_item_aliases();
    Array rstocks;
    for ( int i = 0; i < aliases.size(); i++ ) {
        Variant alias = aliases[i];
        Ref<VDInventoryStock> stock = this->item_stocks[alias];
        rstocks.append ( stock );
        // rtextures.push_back(textures[i].get_ref_ptr());
    }
    return rstocks;
}

void VDInventory::set_amount_usage_flag ( AmountUsageFlag flag ) {
    this->amountUsage = flag;
}

VDInventory::AmountUsageFlag VDInventory::get_amount_usage_flag() const {
    return this->amountUsage;
}

// VDInventoryStocks


void VDInventoryStock::_bind_methods() {
    ClassDB::bind_method ( D_METHOD ( "set_amount_usage_flag", "flag" ), &VDInventoryStock::set_amount_usage_flag );
    ClassDB::bind_method ( D_METHOD ( "get_amount_usage_flag" ), &VDInventoryStock::get_amount_usage_flag );
    ClassDB::bind_method ( D_METHOD ( "set_custom_usage_flag", "flag" ), &VDInventoryStock::set_custom_usage_flag );
    ClassDB::bind_method ( D_METHOD ( "get_custom_usage_flag" ), &VDInventoryStock::get_custom_usage_flag );

    ClassDB::bind_method ( D_METHOD ( "set_stock_alias", "alias" ), &VDInventoryStock::set_stock_alias );
    ClassDB::bind_method ( D_METHOD ( "get_stock_alias" ), &VDInventoryStock::get_stock_alias );
    ClassDB::bind_method ( D_METHOD ( "set_item_object", "item" ), &VDInventoryStock::set_item_object );
    ClassDB::bind_method ( D_METHOD ( "get_item_object" ), &VDInventoryStock::get_item_object );
    ClassDB::bind_method ( D_METHOD ( "set_item_amount", "amount" ), &VDInventoryStock::set_item_amount );
    ClassDB::bind_method ( D_METHOD ( "get_item_amount" ), &VDInventoryStock::get_item_amount );
    ClassDB::bind_method ( D_METHOD ( "get_owning_inventory" ), &VDInventoryStock::get_owning_inventory );

    ADD_PROPERTY ( PropertyInfo ( Variant::STRING, "stock_alias" ), "set_stock_alias", "get_stock_alias" );
    ADD_PROPERTY ( PropertyInfo ( Variant::INT, "amountUsage" ), "set_amount_usage_flag", "get_amount_usage_flag" );
    ADD_PROPERTY ( PropertyInfo ( Variant::INT, "customUsage" ), "set_custom_usage_flag", "get_custom_usage_flag" );

    ADD_PROPERTY ( PropertyInfo ( Variant::OBJECT, "item_object" ), "set_item_object", "get_item_object" );
    ADD_PROPERTY ( PropertyInfo ( Variant::INT, "item_amount" ), "set_item_amount", "get_item_amount" );

    ADD_SIGNAL ( MethodInfo ( "stock_amount_changed", PropertyInfo ( Variant::INT, "new_amount" ), PropertyInfo ( Variant::INT, "old_amount" ) ) );
}

void VDInventoryStock::set_amount_usage_flag ( VDInventory::AmountUsageFlag flag ) {
    this->amountUsage = flag;
}

VDInventory::AmountUsageFlag VDInventoryStock::get_amount_usage_flag() const {
    return this->amountUsage;
}

void VDInventoryStock::set_custom_usage_flag ( VDInventoryStock::CustomAmountUsage flag ) {
    this->customUsage = flag;
}

VDInventoryStock::CustomAmountUsage VDInventoryStock::get_custom_usage_flag() const {
    return this->customUsage;
}

VDInventoryStock::VDInventoryStock() {
    this->item_object = Variant();
    this->item_amount = 1;
}

void VDInventoryStock::set_stock_alias ( String alias ) {
    ERR_FAIL_COND_MSG ( this->owning_inventory.is_valid(), "Cannot set alias if stock item is owned." );
    this->stock_alias = alias;
}

String VDInventoryStock::get_stock_alias() const {
    return this->stock_alias;
}

void VDInventoryStock::set_item_object ( const Variant & item ) {
    this->item_object = item;
}

Variant VDInventoryStock::get_item_object() const {
    return this->item_object;
}

void VDInventoryStock::set_item_amount ( int amount ) {
    if ( amount != this->item_amount ) {
        int old_amount = this->item_amount;
        this->item_amount = amount;
        emit_signal ( "stock_amount_changed", amount, old_amount );
    }
}

int VDInventoryStock::get_item_amount() const {
    return this->item_amount;
}

Ref<VDInventory> VDInventoryStock::get_owning_inventory() const {
    return this->owning_inventory;
}
