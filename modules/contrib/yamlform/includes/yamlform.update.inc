<?php

/**
 * @file
 * YAML form update hook.
 */

use Drupal\Component\Serialization\Yaml;
use Drupal\Core\Field\BaseFieldDefinition;
use Drupal\yamlform\Entity\YamlForm;

/**
 * Add support for HTML and file attachments to YamlFormEmailHandler.
 */
function yamlform_update_8001(&$sandbox) {
  $messages = [];

  // Update 'yamlform.settings' configuration.
  // Copied from \views_update_8001().
  $settings_config = \Drupal::configFactory()->getEditable('yamlform.settings');
  $default_data = Yaml::decode(file_get_contents(drupal_get_path('module', 'yamlform') . '/config/install/yamlform.settings.yml'));

  $settings_config->clear('mail.default_body');
  $settings_config->set('mail.default_body_text', $default_data['mail']['mail.default_body_text']);
  $settings_config->set('mail.default_body_html', $default_data['mail']['mail.default_body_html']);
  $settings_config->save();

  $messages[] = \Drupal::translation()->translate("Update YAML form settings removed 'mail.default_body' and replaced with 'mail_default_body_text' and 'mail_default_body_html'");

  // Update 'yamlform.yamlform.*' configuration.
  // Copied from \views_update_8001().
  $ids = [];

  $config_factory = \Drupal::configFactory();
  foreach ($config_factory->listAll('yamlform.yamlform.') as $yamlform_config_name) {
    $yamlform_config = $config_factory->getEditable($yamlform_config_name);
    $handlers = $yamlform_config->get('handlers');

    $has_email_handler = FALSE;
    foreach ($handlers as $handler_id => $handler) {
      if ($handler['id'] != 'email') {
        continue;
      }
      $has_email_handler = TRUE;

      $base = "handlers.$handler_id";

      // Get debug setting so that we can move it last.
      $debug = $yamlform_config->get($base . '.settings.debug');
      $yamlform_config->clear($base . '.settings.debug');

      $yamlform_config->set($base . '.settings.html', FALSE);
      $yamlform_config->set($base . '.settings.attachments', FALSE);
      $yamlform_config->set($base . '.settings.debug', $debug);
    }

    if ($has_email_handler) {
      $ids[] = $yamlform_config->get('id');
      $yamlform_config->save(TRUE);
    }
  }

  if (!empty($ids)) {
    $messages[] = t('Updated email handler for YAML forms: @ids', ['@ids' => implode(', ', array_unique($ids))]);
  }

  return implode("\n", $messages);
}

/**
 * Issue #2701113: Rework YAML form submission view to use templates.
 */
function yamlform_update_8002(&$sandbox) {
  $settings_config = \Drupal::configFactory()->getEditable('yamlform.settings');
  $settings_config->clear('display');
  $settings_config->save();
}

/**
 * Issue #2705859  Allow YAML form elements to define custom display formats. Add emptu formats to yamlform.settings.yml.
 */
function yamlform_update_8003(&$sandbox) {
  /** @var \Drupal\yamlform\YamlFormElementManagerInterface $element_manager */
  $element_manager = \Drupal::service('plugin.manager.yamlform.element');
  $element_plugins = $element_manager->getInstances();
  $format = [];
  foreach ($element_plugins as $element_id => $element_plugin) {
    $formats = $element_plugin->getFormats();
    if (empty($formats)) {
      continue;
    }
    if (count($formats) == 1 && isset($formats['value'])) {
      continue;
    }

    $format[$element_id] = '';
  }
  $settings_config = \Drupal::configFactory()->getEditable('yamlform.settings');
  $settings_config->set('format', $format);
  $settings_config->save();
}

/**
 * Issue #2709933: Save export options.
 */
function yamlform_update_8004(&$sandbox) {
  $settings_config = \Drupal::configFactory()->getEditable('yamlform.settings');
  $settings_config->set('export', [
    'delimiter' => ',',
    'header_keys' => 'label',
    'entity_reference_format' => 'link',
    'options_format' => 'compact',
    'options_item_format' => 'label',
  ]);
  $settings_config->save();
}

/**
 * Issue #2712457: Implement Wizard/Paging. Add 'current_page' field to 'yamlform_submission' entities.
 */
function yamlform_update_8005() {
  // Install the definition that this field had in
  // \Drupal\yamlform\Entity\YamlFormSubmission::baseFieldDefinitions()
  // at the time that this update function was written. If/when code is
  // deployed that changes that definition, the corresponding module must
  // implement an update function that invokes
  // \Drupal::entityDefinitionUpdateManager()->updateFieldStorageDefinition()
  // with the new definition.
  $storage_definition = BaseFieldDefinition::create('string')
    ->setLabel(t('Current page'))
    ->setDescription(t('The current wizard page.'))
    ->setSetting('max_length', 128);
  \Drupal::entityDefinitionUpdateManager()
    ->installFieldStorageDefinition('current_page', 'yamlform_submission', 'yamlform', $storage_definition);
}

/**
 * Issue #2712457: Implement Wizard/Paging. Update 'yamlform.settings' and 'yamlform.yamlform.*' configuration.
 */
function yamlform_update_8007() {
  // Update 'yamlform.settings' configuration.
  $settings_config = \Drupal::configFactory()->getEditable('yamlform.settings');
  $settings_config->set('settings.default_wizard_prev_button_label', '< Previous Page');
  $settings_config->set('settings.default_wizard_next_button_label', 'Next Page >');
  $settings_config->save();

  // Update 'yamlform.yamlform.*' configuration.
  $config_factory = \Drupal::configFactory();
  foreach ($config_factory->listAll('yamlform.yamlform.') as $yamlform_config_name) {
    $yamlform_config = $config_factory->getEditable($yamlform_config_name);
    $yamlform_config->set('settings.wizard_progress_bar', TRUE);
    $yamlform_config->set('settings.wizard_prev_button_label', '');
    $yamlform_config->set('settings.wizard_next_button_label', '');
    $yamlform_config->save(TRUE);
  }
}

/**
 * Issue #2712463: Provide Entity–attribute–value model for submissions. Create 'yamlform_submission_data' table.
 */
function yamlform_update_8008() {
  $schema = yamlform_schema();
  \Drupal::database()->schema()->createTable('yamlform_submission_data', $schema['yamlform_submission_data']);
}

/**
 * Issue #2712463: Provide Entity–attribute–value model for submissions. Populate 'yamlform_submission_data' table using batch process.
 */
function yamlform_update_8009(&$sandbox) {
  // @see https://api.drupal.org/api/drupal/core%21lib%21Drupal%21Core%21Extension%21module.api.php/function/hook_update_N/8.1.x
  // Loop through 100 YAML form submission at at time.
  $limit = 100;
  if (!isset($sandbox['progress'])) {
    $sandbox['progress'] = 0;
    $sandbox['current_sid'] = 0;
    $sandbox['max'] = \Drupal::database()->query('SELECT COUNT(sid) FROM {yamlform_submission}')->fetchField();
  }

  // @see \Drupal\yamlform\Entity\YamlFormSubmission::save().
  $records = \Drupal::database()->select('yamlform_submission', 's')
    ->fields('s', ['sid', 'yamlform_id', 'data'])
    ->condition('sid', $sandbox['current_sid'], '>')
    ->range(0, $limit)
    ->orderBy('sid', 'ASC')
    ->execute();
  foreach ($records as $record) {
    $data = Yaml::decode($record->data);
    $yamlform_id  = $record->yamlform_id;
    $sid = $record->sid;

    $rows = [];
    $update_submission_record = FALSE;
    foreach ($data as $name => &$item) {
      if (is_array($item)) {
        foreach ($item as $key => $value) {
          // Remove target_id from saved entity_autocomplete fields.
          if (is_array($value) && isset($value['target_id'])) {
            $value = $value['target_id'];
            $item[$key] = $value;
            $update_submission_record = TRUE;
          }

          $rows[] = [
            'yamlform_id' => $yamlform_id,
            'sid' => $sid,
            'name' => $name,
            'delta' => (string) $key,
            'value' => (string) $value,
          ];
        }
      }
      else {
        $rows[] = [
          'yamlform_id' => $yamlform_id,
          'sid' => $sid,
          'name' => $name,
          'delta' => '',
          'value' => (string) $item,
        ];
      }
    }

    // Delete existing submission data rows.
    \Drupal::database()->delete('yamlform_submission_data')
      ->condition('sid', $sid)
      ->execute();

    // Insert new submission data rows.
    $query = \Drupal::database()
      ->insert('yamlform_submission_data')
      ->fields(['yamlform_id', 'sid', 'name', 'delta', 'value']);
    foreach ($rows as $row) {
      $query->values($row);
    }
    $query->execute();

    // Update submission record.
    if ($update_submission_record) {
      \Drupal::database()
        ->update('yamlform_submission')
        ->fields(['data' => Yaml::encode($data)])
        ->condition('sid', $sid)
        ->execute();
    }

    $sandbox['progress']++;
    $sandbox['current_sid'] = $sid;
  }

  $sandbox['#finished'] = ($sandbox['progress'] >= $sandbox['max']) ? TRUE : ($sandbox['progress'] / $sandbox['max']);

  if ($sandbox['#finished']) {
    return t("Populating the 'yamlform_submission_data' table is complete.");
  }
}

/**
 * Issue #2712463: Provide Entity–attribute–value model for submissions. Remove #default_value target_id.
 */
function yamlform_update_8010() {
  $ids = [];

  $config_factory = \Drupal::configFactory();
  foreach ($config_factory->listAll('yamlform.yamlform.') as $yamlform_config_name) {
    $yamlform_config = $config_factory->getEditable($yamlform_config_name);

    $inputs = $yamlform_config->get('inputs');
    if (preg_match('/-\s+target_id:/', $inputs)) {
      // Have to regex to replace inputs target_id: in YAML because if we parse
      // the inputs YAML all comments and formatting will be lost.
      $inputs = preg_replace('/-\s+target_id: /', '- ', $inputs);

      $yamlform_config->set('inputs', $inputs);
      $yamlform_config->save(TRUE);

      $ids[] = $yamlform_config->get('id');
    }
  }

  if ($ids) {
    return t('Updated YAML forms: @ids', ['@ids' => implode(', ', array_unique($ids))]);
  }
}

/**
 * Issue #2718005: Support Confidential submissions.
 */
function yamlform_update_8011() {
  // Update 'yamlform.settings' configuration.
  $settings_config = \Drupal::configFactory()->getEditable('yamlform.settings');
  $settings_config->set('settings.default_form_confidential_message', 'This form is confidential. You must <a href="[site:login-url]/logout?destination=[current-page:url:relative]">Log out</a> to submit it.');
  $settings_config->save();

  // Update 'yamlform.yamlform.*' configuration.
  $config_factory = \Drupal::configFactory();
  foreach ($config_factory->listAll('yamlform.yamlform.') as $yamlform_config_name) {
    $yamlform_config = $config_factory->getEditable($yamlform_config_name);
    $yamlform_config->set('settings.form_confidential', FALSE);
    $yamlform_config->set('settings.form_confidential_message', '');
    $yamlform_config->save(TRUE);
  }
}

/**
 * Issue #2721539: Wizard page numbers and percentage.
 */
function yamlform_update_8012() {
  // Update 'yamlform.yamlform.*' configuration.
  $config_factory = \Drupal::configFactory();
  foreach ($config_factory->listAll('yamlform.yamlform.') as $yamlform_config_name) {
    $yamlform_config = $config_factory->getEditable($yamlform_config_name);
    $yamlform_config->set('settings.wizard_progress_pages', FALSE);
    $yamlform_config->set('settings.wizard_progress_percentage', FALSE);
    $yamlform_config->save(TRUE);
  }
}

/**
 * Issue #2721711: Add start and complete label to wizard settings.
 */
function yamlform_update_8013() {
  // Update 'yamlform.settings' configuration.
  $settings_config = \Drupal::configFactory()->getEditable('yamlform.settings');
  $settings_config->set('settings.default_wizard_start_label', 'Start');
  $settings_config->set('settings.default_wizard_complete_label', 'Complete');
  $settings_config->save();

  // Update 'yamlform.yamlform.*' configuration.
  $config_factory = \Drupal::configFactory();
  foreach ($config_factory->listAll('yamlform.yamlform.') as $yamlform_config_name) {
    $yamlform_config = $config_factory->getEditable($yamlform_config_name);
    $yamlform_config->set('settings.wizard_start_label', '');
    $yamlform_config->set('settings.wizard_complete', TRUE);
    $yamlform_config->set('settings.wizard_complete_label', '');
    $yamlform_config->save(TRUE);
  }
}

/**
 * Issue #2748699: Change all references from Inputs to Elements.
 */
function yamlform_update_8014() {
  $config_factory = \Drupal::configFactory();

  // Update 'yamlform.settings' configuration.
  $settings_config = \Drupal::configFactory()->getEditable('yamlform.settings');

  $yaml = Yaml::encode($settings_config->getRawData());
  $yaml = preg_replace('/(\s+)inputs: /', '\1elements: ', $yaml);
  $yaml = preg_replace('/(\s+)default_inputs: /', '\1default_elements: ', $yaml);

  $settings_config->setData(Yaml::decode($yaml));
  $settings_config->save();

  // Update 'yamlform.yamlform.*' configuration.
  $yamlform_config_names = [
    'example_inputs' => 'example_elements',
    'example_inputs_extras' => 'example_elements_extras',
    'example_inputs_formats' => 'example_elements_formats',
    'example_inputs_masks' => 'example_elements_masks',
    'example_inputs_states' => 'example_elements_states',
  ];
  foreach ($config_factory->listAll('yamlform.yamlform.') as $yamlform_config_name) {
    $yamlform_config = $config_factory->getEditable($yamlform_config_name);
    $yamlform_id = str_replace('yamlform.yamlform.', '', $yamlform_config_name);

    $yaml = Yaml::encode($yamlform_config->getRawData());
    $yaml = preg_replace('/(\s+)inputs: /', '\1elements: ', $yaml);
    $yaml = preg_replace('/(\s+)excluded_inputs: /', '\1excluded_elements: ', $yaml);
    $yaml = str_replace('_inputs_', '_elements_', $yaml);
    $yaml = str_replace(' input ', ' element ', $yaml);
    $yaml = str_replace(' Inputs', ' Elements', $yaml);
    $yaml = str_replace(' inputs', ' elements', $yaml);
    $yaml = str_replace('Private input', 'Private element ', $yaml);

    if (isset($yamlform_config_names[$yamlform_id])) {
      $yaml = str_replace($yamlform_id, $yamlform_config_names[$yamlform_id], $yaml);
      $data = Yaml::decode($yaml);
      \Drupal::configFactory()->getEditable('yamlform.yamlform.' . $yamlform_id)->delete();
      \Drupal::configFactory()->getEditable('yamlform.yamlform.' . $yamlform_config_names[$yamlform_id])->setData($data)->save();
    }
    else {
      $data = Yaml::decode($yaml);
      \Drupal::configFactory()->getEditable('yamlform.yamlform.' . $yamlform_id)->setData($data)->save();
    }
  }
}

/**
 * Issue #2749063: Load YAML form submission data using EAV table. Fix yamlform_submission_data table's deltas.
 */
function yamlform_update_8015(&$sandbox) {
  // @see https://api.drupal.org/api/drupal/core%21lib%21Drupal%21Core%21Extension%21module.api.php/function/hook_update_N/8.1.x
  // Loop through 100 YAML form submission at at time.
  $limit = 100;
  if (!isset($sandbox['progress'])) {
    $sandbox['progress'] = 0;
    $sandbox['current_sid'] = 0;
    $sandbox['max'] = \Drupal::database()->query('SELECT COUNT(sid) FROM {yamlform_submission}')->fetchField();
  }

  // @see \Drupal\yamlform\Entity\YamlFormSubmission::save().
  $records = \Drupal::database()->select('yamlform_submission', 's')
    ->fields('s', ['sid', 'yamlform_id', 'data'])
    ->condition('sid', $sandbox['current_sid'], '>')
    ->range(0, $limit)
    ->orderBy('sid', 'ASC')
    ->execute();
  foreach ($records as $record) {
    $data = Yaml::decode($record->data);
    $yamlform_id  = $record->yamlform_id;
    $sid = $record->sid;

    $rows = [];
    $update_submission_record = FALSE;
    foreach ($data as $name => &$item) {
      if (is_array($item)) {
        $index = 0;
        foreach ($item as $key => $value) {
          // Fix multi value element's delta to be an index.
          // Change ['value1' => 'value1', 'value2' => 'value2'] to
          // [0 => 'value1', 1 => 'value2'].
          if ($key == $value) {
            $key = $index;
          }
          $index++;
          $rows[] = [
            'yamlform_id' => $yamlform_id,
            'sid' => $sid,
            'name' => $name,
            'delta' => (string) $key,
            'value' => (string) $value,
          ];
        }
      }
      else {
        $rows[] = [
          'yamlform_id' => $yamlform_id,
          'sid' => $sid,
          'name' => $name,
          'delta' => '',
          'value' => (string) $item,
        ];
      }
    }

    // Delete existing submission data rows.
    \Drupal::database()->delete('yamlform_submission_data')
      ->condition('sid', $sid)
      ->execute();

    // Insert new submission data rows.
    $query = \Drupal::database()
      ->insert('yamlform_submission_data')
      ->fields(['yamlform_id', 'sid', 'name', 'delta', 'value']);
    foreach ($rows as $row) {
      $query->values($row);
    }
    $query->execute();

    // Update submission record.
    if ($update_submission_record) {
      \Drupal::database()
        ->update('yamlform_submission')
        ->fields(['data' => Yaml::encode($data)])
        ->condition('sid', $sid)
        ->execute();
    }

    $sandbox['progress']++;
    $sandbox['current_sid'] = $sid;
  }

  $sandbox['#finished'] = ($sandbox['progress'] >= $sandbox['max']) ? TRUE : ($sandbox['progress'] / $sandbox['max']);

  if ($sandbox['#finished']) {
    return t("Populating the 'yamlform_submission_data' table is complete.");
  }
}

/**
 * Issue #2749063: Load YAML form submission data using EAV table. Remove yamlform_submission.data field.
 */
function yamlform_update_8016() {
  db_drop_field('yamlform_submission', 'data');
}

/**
 * Issue #2748699: Change all references from Inputs to Elements. Update yamlform_id in the yamlform_submission_data table.
 */
function yamlform_update_8017() {
  $yamlform_config_names = [
    'example_inputs' => 'example_elements',
    'example_inputs_extras' => 'example_elements_extras',
    'example_inputs_formats' => 'example_elements_formats',
    'example_inputs_masks' => 'example_elements_masks',
    'example_inputs_states' => 'example_elements_states',
  ];
  foreach ($yamlform_config_names as $config_name_search => $config_name_replace) {
    db_query('UPDATE {yamlform_submission_data} SET yamlform_id=:new_id WHERE yamlform_id=:old_id', [':old_id' => $config_name_search, ':new_id' => $config_name_replace]);
  }
}

/**
 * Issue #2752817: Create creditcard composite element.
 */
function yamlform_update_8018() {
  $config_factory = \Drupal::configFactory();

  // Update 'yamlform.settings' configuration.
  $settings_config = \Drupal::configFactory()->getEditable('yamlform.settings');
  $yaml = Yaml::encode($settings_config->getRawData());
  $yaml = str_replace('creditcard', 'creditcard_number', $yaml);
  $settings_config->setData(Yaml::decode($yaml));
  $settings_config->save();

  // Update 'yamlform.yamlform.*' configuration.
  foreach ($config_factory->listAll('yamlform.yamlform.') as $yamlform_config_name) {
    $yamlform_config = $config_factory->getEditable($yamlform_config_name);
    $yaml = Yaml::encode($yamlform_config->getRawData());
    $yaml = str_replace('creditcard', 'creditcard_number', $yaml);
    $data = Yaml::decode($yaml);
    \Drupal::configFactory()->getEditable($yamlform_config_name)->setData($data)->save();
  }
}

/**
 * Issue #2756643: Allow entity specific submission total and user limits to be set.
 */
function yamlform_update_8019(&$sandbox) {
  // Update 'yamlform.yamlform.*' configuration.
  $config_factory = \Drupal::configFactory();
  foreach ($config_factory->listAll('yamlform.yamlform.') as $yamlform_config_name) {
    $yamlform_config = $config_factory->getEditable($yamlform_config_name);
    $yamlform_config->set('settings.entity_limit_user', NULL);
    $yamlform_config->set('settings.entity_limit_total', NULL);
    $yamlform_config->save(TRUE);
  }
}

/**
 * Issue #2757479: Create dedicated YAML form node module.
 */
function yamlform_update_8020(&$sandbox) {
  // Since the YAML form node configuration files are already installed, we
  // need to manually install the module.
  $extension_config = \Drupal::configFactory()->getEditable('core.extension');
  $extension_config
    ->set("module.yamlform_node", 0)
    ->set('module', module_config_sort($extension_config->get('module')))
    ->save(TRUE);
  drupal_static_reset('system_rebuild_module_data');
  drupal_set_installed_schema_version('yamlform_node', \Drupal::CORE_MINIMUM_SCHEMA_VERSION);
  \Drupal::logger('system')->info('%module module installed.', ['%module' => 'yamlform']);

  return (string) t("The new YAML Form Node module has been enabled.");
}

/**
 * Issue #2757981: Improve YAML form node integration.
 */
function yamlform_update_8021(&$sandbox) {
  // Clear entity type and id that are pointing to the submission's YAML form.
  \Drupal::database()->query("UPDATE {yamlform_submission} SET entity_type=NULL, entity_id=NULL WHERE entity_type='yamlform' AND yamlform_id=entity_id");
}

/**
 * Issue #2764531: Add ownership to YAML form entity.
 */
function yamlform_update_8022(&$sandbox) {
  // Update 'yamlform.yamlform.*' configuration.
  $config_factory = \Drupal::configFactory();
  foreach ($config_factory->listAll('yamlform.yamlform.') as $yamlform_config_name) {
    $yamlform_config = $config_factory->getEditable($yamlform_config_name);
    $yamlform_config->set('uid', NULL);
    $yamlform_config->set('template', FALSE);
    $yamlform_config->save(TRUE);
  }
}

/**
 * Issue #2765057: Remove default elements.
 */
function yamlform_update_8023(&$sandbox) {
  $settings_config = \Drupal::configFactory()->getEditable('yamlform.settings');
  $data = $settings_config->getRawData();

  // Move customized default elements to a template.
  if (md5($data['elements']['default_elements']) != 'c5a931226e09a0d1f30a032d587bceb4') {
    YamlForm::create([
      'id' => 'template_default_elements',
      'title' => 'Template: Default Elements',
      'description' => 'Default elements previously used when a new form is created.<br/>For more information see: Issue <a href="https://www.drupal.org/node/2765057">#2765057</a>: Remove default elements.',
      'template' => TRUE,
      'elements' => $data['elements']['default_elements'],
    ])->save();
    $customized_default_elements = TRUE;
  }
  else {
    $customized_default_elements = FALSE;
  }

  // Remove default elements.
  unset($data['elements']['default_elements']);
  $settings_config->setData($data);
  $settings_config->save();

  // Return message is custom default elements exist.
  if ($customized_default_elements) {
    return t("Moved existing default elements to a new YAML form template called 'Template: Default Elements' (template_default_elements)");
  }
  else {
    return NULL;
  }
}

/**
 * Issue #2765831: Improve text field autocompletion support.
 */
function yamlform_update_8024() {
  $config_factory = \Drupal::configFactory();
  foreach ($config_factory->listAll('yamlform.yamlform.') as $yamlform_config_name) {
    $yamlform_config = $config_factory->getEditable($yamlform_config_name);
    $yaml = Yaml::encode($yamlform_config->getRawData());
    if (strpos($yaml, "'#autocomplete'") !== FALSE) {
      $yaml = str_replace("'#autocomplete'", "'#autocomplete_options'", $yaml);
      $data = Yaml::decode($yaml);
      $yamlform_config->setData($data)->save();
    }
  }
}

/**
 * Issue #2766453: Add sticky and notes to submissions.
 */
function yamlform_update_8025(&$sandbox) {
  // @see https://www.drupal.org/node/2554097
  // From: \Drupal\yamlform\Entity\YamlFormSubmission::baseFieldDefinitions
  $definitions = [];
  $definitions['sticky'] = BaseFieldDefinition::create('boolean')
    ->setLabel(t('Sticky'))
    ->setDescription(t('A flag that indicate the status of the YAML form submission.'))
    ->setDefaultValue(FALSE);
  $definitions['notes'] = BaseFieldDefinition::create('string_long')
    ->setLabel(t('Notes'))
    ->setDescription(t('Administrative notes about the YAML form submission.'))
    ->setDefaultValue('');
  foreach ($definitions as $name => $definition) {
    \Drupal::entityDefinitionUpdateManager()
      ->installFieldStorageDefinition($name, 'yamlform_submission', 'yamlform_submission', $definition);
  }
}

/**
 * Issue #2767637: Allow table columns to be customized.
 */
function yamlform_update_8026(&$sandbox) {
  // Remove broken YAML form export settings.
  db_query("DELETE FROM {key_value} WHERE collection='state' AND name LIKE 'yamlform%'");
}

/**
 * Issue #2767891: Allow dialogs to be disabled.
 */
function yamlform_update_8027(&$sandbox) {
  // Update 'yamlform.settings' configuration.
  $settings_config = \Drupal::configFactory()->getEditable('yamlform.settings');
  $settings_config->set('ui.dialog_disabled', FALSE);
  $settings_config->save();
}

/**
 * Issue #2770823: Save open/close state of detail form element.
 */
function yamlform_update_8028(&$sandbox) {
  // Update 'yamlform.settings' configuration.
  $settings_config = \Drupal::configFactory()->getEditable('yamlform.settings');
  $settings_config->set('ui.details_save', TRUE);
  $settings_config->save();
}

/**
 * Issue #2770071 by jrockowitz: Compatibility with SMTP authentication.
 */
function yamlform_update_8029(&$sandbox) {
  /** @var \Drupal\yamlform\YamlFormEmailProviderInterface $email_provider */
  $email_provider = \Drupal::service('yamlform.email_provider');
  $email_provider->check();
}

/**
 * Issue #2773325: Remove captcha formatting.
 */
function yamlform_update_8030(&$sandbox) {
  // Update 'yamlform.settings' configuration.
  $settings_config = \Drupal::configFactory()->getEditable('yamlform.settings');
  $settings_config->clear('format.captcha');
  $settings_config->save();
}

/**
 * Issue #2772697: Option to disable HTML5 validations.
 */
function yamlform_update_8031() {
  // Update 'yamlform.settings' configuration.
  $settings_config = \Drupal::configFactory()->getEditable('yamlform.settings');
  $settings_config->set('settings.default_form_novalidate', FALSE);
  $settings_config->save();

  // Update 'yamlform.yamlform.*' configuration.
  $config_factory = \Drupal::configFactory();
  foreach ($config_factory->listAll('yamlform.yamlform.') as $yamlform_config_name) {
    $yamlform_config = $config_factory->getEditable($yamlform_config_name);
    $yamlform_config->set('settings.form_novalidate', FALSE);
    $yamlform_config->save(TRUE);
  }
}

/**
 * Issue #2775181 by jrockowitz: Address fatal error when handler plugin is missing.
 */
function yamlform_update_8032() {
  // Update 'yamlform.yamlform.*' configuration.
  $config_factory = \Drupal::configFactory();
  foreach ($config_factory->listAll('yamlform.yamlform.') as $yamlform_config_name) {
    $yamlform_config = $config_factory->getEditable($yamlform_config_name);
    $yamlform_config->set('dependencies', []);
    $yamlform_config->save(TRUE);
  }
}

/**
 * Issue #2778521: Refactor email_confirm element to support customization.
 */
function yamlform_update_8033() {
  $config_factory = \Drupal::configFactory();
  // Update 'yamlform.yamlform.*' configuration.
  foreach ($config_factory->listAll('yamlform.yamlform.') as $yamlform_config_name) {
    $yamlform_config = $config_factory->getEditable($yamlform_config_name);
    $yaml = Yaml::encode($yamlform_config->getRawData());
    $yaml = str_replace('#confirm_title', '#confirm__title', $yaml);
    $data = Yaml::decode($yaml);
    \Drupal::configFactory()->getEditable($yamlform_config_name)->setData($data)->save();
  }
}

/**
 * Issue #2778715: Refactor other elements.
 */
function yamlform_update_8034() {
  $config_factory = \Drupal::configFactory();
  // Update 'yamlform.yamlform.*' configuration.
  foreach ($config_factory->listAll('yamlform.yamlform.') as $yamlform_config_name) {
    $yamlform_config = $config_factory->getEditable($yamlform_config_name);
    $yaml = Yaml::encode($yamlform_config->getRawData());
    $yaml = str_replace('#other_option_label', '#other__option_label', $yaml);
    $yaml = str_replace('#other_placeholder', '#other__placeholder', $yaml);
    $yaml = str_replace('#other_description', '#other__description', $yaml);
    $data = Yaml::decode($yaml);
    \Drupal::configFactory()->getEditable($yamlform_config_name)->setData($data)->save();
  }
}

/**
 * Issue #2778857: Description of elements.
 */
function yamlform_update_8035() {
  // Update 'yamlform.settings' configuration.
  $settings_config = \Drupal::configFactory()->getEditable('yamlform.settings');
  $settings_config->set('elements.default_description_display', '');
  $settings_config->save();
}

/**
 * Issue #2781663: Add URL with message to YAML form confirmation types.
 */
function yamlform_update_8036() {
  $config_factory = \Drupal::configFactory();
  // Update 'yamlform.yamlform.*' configuration.
  foreach ($config_factory->listAll('yamlform.yamlform.') as $yamlform_config_name) {
    $yamlform_config = $config_factory->getEditable($yamlform_config_name);
    $yaml = Yaml::encode($yamlform_config->getRawData());
    $yaml = str_replace('confirmation_type: url', 'confirmation_type: url_message', $yaml);
    $data = Yaml::decode($yaml);
    \Drupal::configFactory()->getEditable($yamlform_config_name)->setData($data)->save();
  }
}

/**
 * Issue #2781713: Remove id, title, and description from setting.
 */
function yamlform_update_8037() {
  // Update 'yamlform.yamlform.*' configuration.
  $config_factory = \Drupal::configFactory();
  foreach ($config_factory->listAll('yamlform.yamlform.') as $yamlform_config_name) {
    $yamlform_config = $config_factory->getEditable($yamlform_config_name);
    $yamlform_config->clear('settings.id');
    $yamlform_config->clear('settings.title');
    $yamlform_config->clear('settings.description');
    $yamlform_config->save(TRUE);
  }
}

/**
 * Issue #2783527: Order YAML form settings when saved and apply defaults.
 */
function yamlform_update_8038() {
  _yamlform_update_settings();
}

/**
 * Issue #2783575: Add autofocus form setting.
 */
function yamlform_update_8039() {
  _yamlform_update_settings();
}

/**
 * Issue #2783361: Replace embedded videos by links to them.
 */
function yamlform_update_8040() {
  // Update 'yamlform.settings' configuration.
  $settings_config = \Drupal::configFactory()->getEditable('yamlform.settings');
  $settings_config->set('ui.video_display', 'dialog');
  $settings_config->save();
}

/**
 * Issue #2783771: Add setting for #allowed_tags.
 */
function yamlform_update_8041() {
  // Update 'yamlform.settings' configuration.
  $settings_config = \Drupal::configFactory()->getEditable('yamlform.settings');
  $settings_config->set('elements.allowed_tags', 'admin');
  $settings_config->save();
}

/**
 * Issue #2783855: Date field display and validation.
 */
function yamlform_update_8042() {
  $config_factory = \Drupal::configFactory();
  // Update 'yamlform.yamlform.*' configuration.
  foreach ($config_factory->listAll('yamlform.yamlform.') as $yamlform_config_name) {
    $yamlform_config = $config_factory->getEditable($yamlform_config_name);
    $yaml = Yaml::encode($yamlform_config->getRawData());
    $yaml = str_replace('#date_format', '#date_date_format', $yaml);
    $data = Yaml::decode($yaml);
    \Drupal::configFactory()
      ->getEditable($yamlform_config_name)
      ->setData($data)
      ->save();
  }
}

/**
 * Issue #2783785: Add html editor to UI.
 */
function yamlform_update_8043(&$sandbox) {
  // Update 'yamlform.settings' configuration.
  $settings_config = \Drupal::configFactory()->getEditable('yamlform.settings');
  $settings_config->set('ui.html_editor_disabled', FALSE);
  $settings_config->save();
}
