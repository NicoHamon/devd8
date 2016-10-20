<?php

namespace Drupal\consolemodule\Plugin\Block;

use Drupal\Core\Block\BlockBase;
use Drupal\Core\Form\FormStateInterface;

/**
 * Provides a 'consoleblock' block.
 *
 * @Block(
 *  id = "consoleblock",
 *  admin_label = @Translation("Consoleblock"),
 * )
 */
class consoleblock extends BlockBase {

  /**
   * {@inheritdoc}
   */
  public function blockForm($form, FormStateInterface $form_state) {
    $form['test'] = array(
      '#type' => 'textfield',
      '#title' => $this->t('test'),
      '#description' => $this->t(''),
      '#default_value' => isset($this->configuration['test']) ? $this->configuration['test'] : '',
      '#maxlength' => 64,
      '#size' => 64,
      '#weight' => '0',
    );

    return $form;
  }

  /**
   * {@inheritdoc}
   */
  public function blockSubmit($form, FormStateInterface $form_state) {
    $this->configuration['test'] = $form_state->getValue('test');
  }

  /**
   * {@inheritdoc}
   */
  public function build() {
    $build = [];
    $build['consoleblock_test']['#markup'] = '<p>' . $this->configuration['test'] . '</p>';

    return $build;
  }

}
