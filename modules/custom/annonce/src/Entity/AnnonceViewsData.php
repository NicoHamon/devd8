<?php

namespace Drupal\annonce\Entity;

use Drupal\views\EntityViewsData;
use Drupal\views\EntityViewsDataInterface;

/**
 * Provides the views data for the Annonce entity type.
 */
class AnnonceViewsData extends EntityViewsData implements EntityViewsDataInterface {
  /**
   * {@inheritdoc}
   */
  public function getViewsData() {
    $data = parent::getViewsData();

    $data['annonce_history']['table']['base'] = array(
      'field' => 'hid',
      'title' => t('Annonce history'),
      'help' => t('The annonce history'),
    );

      $data['annonce_history']['aid'] = array(
          'title' => t('Annonce ID'),
          'help' => t('Annonce\'s ID'),
          'field' => array(
              // ID of field handler plugin to use.
              'id' => 'numeric',
          ),
         /* 'sort' => array(
              // ID of sort handler plugin to use.
              'id' => 'standard',
          ),
          'filter' => array(
              // ID of filter handler plugin to use.
              'id' => 'numeric',
          ),
          'argument' => array(
              // ID of argument handler plugin to use.
              'id' => 'numeric',
          ),*/
      );

      $data['annonce_history']['uid'] = array(
            'title' => t('User\'s ID'),
            'field' => array(
                  // ID of field handler plugin to use.
                  'id' => 'numeric',
              ),
           /* 'sort' => array(
                  // ID of sort handler plugin to use.
                  'id' => 'standard',
              ),
            'filter' => array(
                  // ID of filter handler plugin to use.
                  'id' => 'numeric',
              ),
            'argument' => array(
                  // ID of argument handler plugin to use.
                  'id' => 'numeric',
              ),*/
          'relationship' => array(
              // Views name of the table to join to for the relationship.
              'base' => 'users_field_data',
              // Database field name in the other table to join on.
              'base field' => 'uid',
              // ID of relationship handler plugin to use.
              'id' => 'standard',
              // Default label for relationship in the UI.
              'label' => t('users id'),
          ),
      );

    $data['annonce_history']['table']['join'] = array(

        'users_field_data' => array(
            // Primary key field in node_field_data to use in the join.
            'left_field' => 'uid',
            // Foreign key field in example_table to use in the join.
            'field' => 'uid',
        ),
    );

      $data['annonce_history']['update_time'] = array(
          'title' => t('Update time'),
          'help' => t('Time of update\'s annonce'),
          'field' => array(
              // ID of field handler plugin to use.
              'id' => 'date',
          ),
        /*  'sort' => array(
              // ID of sort handler plugin to use.
              'id' => 'date',
          ),
          'filter' => array(
              // ID of filter handler plugin to use.
              'id' => 'date',
          ),*/
      );


      $data['annonce_history']['table']['group'] = t('group history annonce');
    return $data;
  }

}
