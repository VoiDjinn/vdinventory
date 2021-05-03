#ifndef VDINVENTORY_H
#define VDINVENTORY_H

#include "core/variant.h"
#include "core/reference.h"
#include "../vdcore/VDComponent.h"

class VDIVcInventoryStock;

class VDIVcInventory : public VDCcComponent
{
    GDCLASS ( VDIVcInventory, VDCcComponent );

    Ref<VDIVcInventoryStock> create_item_stock ( const Variant &item, String alias );

protected:
    static void _bind_methods();

    HashMap<String, Ref<VDIVcInventoryStock>> item_stocks;

    virtual Ref<VDIVcInventoryStock> stock_creation ( const Variant &item, String alias );

public:
    VDIVcInventory();

    enum AmountUsageFlag {
        KEEP_ON_ZERO,
        KEEP_ON_NEGATIVE,
        REMOVE_ON_ZERO,
        REMOVE_ON_NEGATIVE,

        KEEP_NON_POSITIVE = KEEP_ON_ZERO || KEEP_ON_NEGATIVE,
        REMOVE_NON_POSITIVE = REMOVE_ON_ZERO || REMOVE_ON_NEGATIVE
    };

protected:
    AmountUsageFlag amountUsage = AmountUsageFlag::REMOVE_NON_POSITIVE;

public:
    Ref<VDIVcInventoryStock> set_item_stock ( const Variant &item, String id, int amount = 1 );
    void remove_item_stock ( String id );
    void transfer_item ( Ref<VDIVcInventory> other_inventory, String id, int amount, bool strict_transfer = true );

    Ref<VDIVcInventoryStock> get_item_stock ( String id );
    int get_item_stock_amount ( String id );
    Variant get_item_stock_object ( String item_id );

    List<String> get_item_aliases();
    Array get_item_stocks();
    void set_amount_usage_flag ( AmountUsageFlag flag );
    AmountUsageFlag get_amount_usage_flag() const;
};

VARIANT_ENUM_CAST ( VDIVcInventory::AmountUsageFlag )

// VDInventoryStocks

class VDIVcInventoryStock : public Reference
{
    GDCLASS ( VDIVcInventoryStock, Reference );

    friend class VDIVcInventory;

    String stock_alias;
    Ref<VDIVcInventory> owning_inventory;

public:
    enum CustomAmountUsage {
        FROM_INVENTORY,
        OVERWRITE
    };

    VDIVcInventory::AmountUsageFlag amountUsage = VDIVcInventory::AmountUsageFlag::REMOVE_NON_POSITIVE;
    CustomAmountUsage customUsage = CustomAmountUsage::FROM_INVENTORY;

protected:
    static void _bind_methods();

    Variant item_object;
    int item_amount;

public:
    VDIVcInventoryStock();

    void set_amount_usage_flag ( VDIVcInventory::AmountUsageFlag flag );
    VDIVcInventory::AmountUsageFlag get_amount_usage_flag() const;
    void set_custom_usage_flag ( CustomAmountUsage flag );
    CustomAmountUsage get_custom_usage_flag() const;

    void set_stock_alias ( String alias );
    String get_stock_alias() const;
    void set_item_object ( const Variant & item );
    Variant get_item_object() const;
    void set_item_amount ( int amount );
    int get_item_amount() const;
    Ref<VDIVcInventory> get_owning_inventory() const;
};

VARIANT_ENUM_CAST ( VDIVcInventoryStock::CustomAmountUsage )

#endif
