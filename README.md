# UC Product Pickup Address


## COMPATIBILITY

Drupal 7.x Ubercart 3.x

## ABOUT

UC Pickup Address is a Drupal Ubercart module that adds an admin page to change all of the "pickup addresses" on a product at the same time.

## INSTALLATION

Install as usual in the sites/all/modules folder

## USAGE

Menu: Home » Administration » Store » Products » Product Pickup Addresses

Settings: /admin/store/products/uc_pickup_address

To mass change a product from one pickup address to another, do the following
1) Edit one of the products that will change. Any one of them is fine
2) vist the Product Pickup Address admin page /admin/store/products/uc_pickup_address
3) Look for the OLD address, in the list
4) Click the link next to the old address, for example: [List 16 products]()
5) Put a checkmark next to the products that will need an address update, or click the select all checkbox at the top.
6) In the **Select Address** dropdown, choose the new address
7) Click the [Change product pickup address] button
8) Done!

## UX EXPECTATION

If you move all products out of an address, it will automatically DELETE that address from the table. One product must remain with an address to keep it in the table, otherwise it's gone for good.

## KNOWN ISSUES
v1.6 has a bug when switching between countries. It does not respect the State\Province change. Make sure you verify the correct State\Province before saving if you are switching countries.
