<?php
/**
 * @file
 * Ubercart Pickup Address Manager
 *
 */
 
/**
 * Form builder to set product pickup address settings.
 */
function uc_pickup_address_admin_form($form, &$form_state) {
  $form = array();
  
  drupal_set_title(t('Product Pickup Addresses'));
  
  // load addresses of products
  $addresses= _uc_pickup_address_load_addresses();
  $store_address = _uc_pickup_address_get_store_address();
  if (is_object($store_address)) {
    if (!in_array($store_address, $addresses)) {
      $addresses[] = $store_address;
    }
  }
  $noaddress_count = _uc_pickup_address_get_address_count(NULL);
  $hash_table = array();
  
  if (count($addresses) || $noaddress_count) {
    $header = array(t('Address'), t('List products with this address'));
    $rows = array();
    // products with address
    if (count($addresses)) {
      foreach ($addresses as $address) {
      
      
        $hash = md5(print_r($address, TRUE));
        $address_count = _uc_pickup_address_get_address_count($address);
        $title = format_plural($address_count, 'List 1 product', 'List @count products');
        // avoid duplicate hash values
        if (!isset($hash_table[$hash])) {
          $hash_table[$hash] = TRUE;
          $rows[] = array(
            '' . $address,
            l($title, "admin/store/products/uc_pickup_address/$hash"),
          );
        }
        else {
          // @todo handle duplicate hash values
          $rows[] = array(
            '' . $address,
            $title,
          );
        }
      }
    }
    // products with no address
    if ($noaddress_count) {
      $hash = md5('');
      $title = format_plural($noaddress_count, 'List 1 product', 'List @count products');
      $rows[] = array(
        t('No address'),
        l($title, "admin/store/products/uc_pickup_address/$hash"),
      );
    }
    $form['info'] = array(
      '#markup' => theme('table', array('header' => $header, 'rows' => $rows)),
    );
  }
  else {
    $form['info'] = array(
      '#type' => 'markup',
      '#value' => t('There are no products.'),
    );
  }

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit changes'),
  );
  return $form;
}

/**
 * Form builder to view products of a certain product pickup address.
 */
function uc_pickup_address_admin_view_form($form, &$form_state, $address_hash) {
  $form = array();
  // load addresses of products
  $addresses= _uc_pickup_address_load_addresses();
  // add store address
  $store_address = _uc_pickup_address_get_store_address();
  if (is_object($store_address)) {
    if (!in_array($store_address, $addresses)) {
      $addresses[] = $store_address;
    }
  }

  // find the address with the given hash
  $address = NULL;
  $no_address = FALSE;
  $other_addresses = array();
  if (count($addresses)) {
    foreach ($addresses as $id => $check_address) {
      $hash = md5(print_r($check_address, TRUE));
      if ($hash==$address_hash) {
        if ($address == NULL) {
          // address found
          $address = $check_address;
          // show address
          $form['address'] = array(
            '#markup' => t('Product Pickup Address') . ':' . '<br />' . $address . '<br />',
          );
        }
      }
      else {
        // copy addresses to another array, except the current address
        // this array is used for the select form element
        $other_addresses[]=$check_address;
      }
    }
  }
  if ($address == NULL) {
    // check "no address"
    if ($address_hash==md5('')) {
      $no_address = TRUE;
      // show "no address"
      $form['address'] = array(
        '#markup' => t('Product Pickup Address') . ':' . '<br />' . t('No address') . '<br />',
      );
      // store in the form that this is not an address,
      // and the submit handler should use insert instead of update
      $form['no_address'] = array(
        '#type' => 'value',
        '#value' => TRUE,
      );
    }
    else {
      // show error message if hash not found
      drupal_set_message(t('Address not found'), 'error');
      return $form;
    }
  }

  // get number of products using this address (or not using address if $address is NULL)
  $address_count = _uc_pickup_address_get_address_count($address);

  if ($address_count<10000) {
    // get products with this address
    $options = array();
    if ($no_address) {
      // query to get nodes with no address
      $query = db_query(
      "SELECT n1.nid as nid FROM {node} n1 
       WHERE type IN (:types) AND NOT EXISTS (
         SELECT n2.nid FROM {uc_quote_product_locations} qpl
         LEFT JOIN {node} n2 ON n2.nid = qpl.nid
         WHERE qpl.nid = n1.nid
       )", array(':types' => uc_product_types()));
       $result = $query->fetchAll();
    }
    else {
      // query to get nodes with the given address
      $query = db_select('uc_quote_product_locations'); 
      $query->addField('uc_quote_product_locations', 'nid'); 
      foreach ($address as $key => $value) {
        if ($key=='email') {
          continue;
        }
        $query->condition($key, $value);
      }
      $result = $query->execute();
    }
    foreach ($result as $record) {
      if ($node = node_load($record->nid)) {
        $options[$node->nid] = array(
          'title' => l($node->title, 'node/' . $node->nid . '/edit'),
        );
      }
    }
    // add table
    $form['table'] = array(
      '#type' => 'tableselect',
      '#header' => array('title' => t('Products')),
      '#options' => $options,
    );
    // add a big gap to avoid accidents
    $form['gap'] = array(
      '#markup' => '<br /><br /><br />',
    );
  }
  // if there are too many products, output only a simple list
  else {
    $form['products'] = array(
      '#markup' => '<br />' . t('Products') . ':' . '<br />',
    );
    // get products with this address
    $options = array();
    if ($no_address) {
      // query to get nodes with no address
      $query = db_query(
      "SELECT n1.nid as nid FROM {node} n1
       WHERE NOT EXISTS (
         SELECT n2.nid FROM {uc_quote_product_locations} qpl
         LEFT JOIN {node} n2 ON n2.nid = qpl.nid
         WHERE qpl.nid = n1.nid
       )");
       $result = $query->fetchAll();
    }
    else {
      $query = db_select('uc_quote_product_locations'); 
      $query->addField('uc_quote_product_locations', 'nid'); 
      foreach ($address as $key => $value) {
        if ($key=='email') {
          continue;
        }
        $query->condition($key, $value);
      }
      $result = $query->execute();
    }
    foreach ($result as $record) {
      if ($node = node_load($record->nid)) {
        // show node title
        $form['node_' . $node->nid] = array(
          '#markup' => l($node->title, 'node/' . $node->nid . '/edit') . '<br />',
        );
        // mimic a selected checkbox when the form is submitted
        $form['table']['#type'] = 'value';
        $form['table']['#value'][$node->nid] = TRUE;
      }
    }
    // add a big gap to avoid accidents
    $form['gap'] = array(
      '#markup' => '<br /><br /><br />',
    );
  }
  
  // construct select form element with other addresses
  if ($no_address) {
    $select_options = array();
  }
  else {
    $select_options = array(0 => t('- Store default -'));
  }
  
  if (count($other_addresses)) {
    foreach ($other_addresses as $other_address) {
      $select_options[drupal_json_encode($other_address)] = $other_address->company . ' ' . $other_address->street1 . ' ' . $other_address->city;
    }
  }
  if (count($select_options)) {
    $select_element = array(
      '#type' => 'select',
      '#title' =>  t('Select address'),
      '#options' => $select_options,
    );
  }
  
  // add select element and a button if there are any options
  if (count($select_options)) {
    $form['address_select'] = $select_element;
        
    // add submit button
    $form['actions'] = array('#type' => 'actions');
    $form['actions']['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Change product pickup address'),
    );
  }
  return $form;
}


function uc_pickup_address_admin_view_form_submit($form, &$form_state) {
  // get the selected address and convert it to address object
  if ($form_state['values']['address_select']) {
    $address_select = drupal_json_decode($form_state['values']['address_select']);
    $selected_address = new UcAddress;
    foreach ($address_select as $key => $value) {
      $selected_address->$key = $value;
    }
  }
  else {
    // address should be changed to store default (deleted)
    $selected_address = NULL;
  }
  
  $changed_count = 0;
  
  // change selected products
  if (isset($form_state['values']['table'])) {
    foreach ($form_state['values']['table'] as $nid => $selected) {
      if ($selected) {
        if ($selected_address!=NULL) {
          // construct the fields array for insert/update
          $fields = array();
          foreach ($address_select as $key => $value) {
            if ($key=='email') {
              continue;
            }
            $fields[$key] = $value;
          }
          if (isset($form_state['values']['no_address'])) {
            // if node has no address, insert it
            $query = db_insert('uc_quote_product_locations');
            $fields['nid'] = $nid;
          }
          else {
            // if overwriting an existing address, update it
            $query = db_update('uc_quote_product_locations');
            $query->condition('nid', $nid);
          }
          $query->fields($fields);
        }
        else {
          // if setting an existing address to store default, delete it
          $query = db_delete('uc_quote_product_locations');
          $query->condition('nid', $nid);
        }
        $query->execute();
        $changed_count++;
      }
    }
  }
  if ($changed_count==0) {
    drupal_set_message(t('No changes'));
  }
  elseif ($changed_count==1) {
    drupal_set_message(t('Changed 1 product'));
  }
  else {
    drupal_set_message(t('Changed @count products', array('@count' => $changed_count)));
  }
  // redirect to the admin form and list current addresses
  $form_state['redirect'] = 'admin/store/products/uc_pickup_address';
}
