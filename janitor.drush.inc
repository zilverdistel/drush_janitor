<?php 
/**
 * @file
 * Drush commands for janitor.
 */

/**
 * Implements hook_drush_command().
 */
function janitor_drush_command() {
  $items = array();
  
  $items['janitor-remove'] = array(
    'description' => dt('This command presents an overview of how crippled your drupal installation might be.'),
    'aliases' => array('janitor', 'jr'),
  );
  
  return $items;
}

/**
 * Implements hook_drush_help().
 */
function janitor_drush_help($section) {
  switch ($section) {
    case 'drush:janitor-remove':
      return dt('Need help with the janitor-overview command?');
  }
}

/**
 * Callback function for the janitor-overview command.
 */
function drush_janitor_remove() {
  $extensions = drush_get_extensions();  
  $projects = drush_get_projects();
  
  $can_be_removed = array();
  $disabled_but_not_uninstalled = array();
  foreach ($extensions as $extension) {
    // We skip all core extensions
    if ($extension->info['project'] == 'drupal') {
      continue;
    }    
    
    // If the status is 1, we skip it (it can happen to custom modules (they don't all appear in $projects it seams)
    if ($extension->status == 1) {
      continue;
    }
    
    // if the corresponding project is enabled
    if (array_key_exists($extension->info['project'], $projects) 
        && $projects[$extension->info['project']]['status']
      ) {
      continue;
    }
    
    if ($extension->schema_version >= 0) {
      $disabled_but_not_uninstalled[$extension->name] = $extension;
      continue;
    }
    
    if (!empty($extension->info['dependents'])) {
      // If none of the dependents are enabled, we can safely remove this one,
      // this shouldn't be the case in any way, just making triple sure!
      foreach ($extension->info['dependents'] as $dependent) {
        if (array_key_exists($dependent, $extensions) && $extensions[$dependent]->status == 1) {
          die("Something is wrong: The extension {$extension->name} is disabled, but one of its dependents ($dependent) is still enabled.");
        }
      }
    }
    
    // only 'top-level' projects are considered
    if ($extension->name != $extension->info['project']) {
      continue;
    }
    
    $can_be_removed[$extension->name] = $extension; 
    $can_be_removed[$extension->name]->path = drupal_get_path($extension->type, $extension->name);
  }
    
  if (!empty($disabled_but_not_uninstalled)) {
    drush_print('These projects must be uninstalled before they can be removed: ');
    drush_print(implode(' ', array_keys($disabled_but_not_uninstalled)));
    drush_print();
  }
  
  if (!empty($can_be_removed)) {
    drush_print('These projects can be safely removed: ');
    drush_print(implode(' ', array_keys($can_be_removed)));
    drush_print();
    drush_print('Here\'s a list of the corresponding file system paths for your convenience: ');
    drush_print(implode(' ', array_map(create_function('$obj', 'return $obj->path;'), $can_be_removed)));    
  }
  else {
    drush_print('No projects have be found that can be removed.');
  }
}
