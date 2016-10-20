<?php

namespace Drupal\consolemodule\Controller;

use Drupal\Core\Controller\ControllerBase;

/**
 * Class ConsoleGeneratedController.
 *
 * @package Drupal\consolemodule\Controller
 */
class ConsoleGeneratedController extends ControllerBase {
  /**
   * Consolemodule.
   *
   * @return string
   *   Return Hello string.
   */
  public function consolemodule() {
    return [
      '#type' => 'markup',
      '#markup' => $this->t('Implement method: consolemodule')
    ];
  }

}
