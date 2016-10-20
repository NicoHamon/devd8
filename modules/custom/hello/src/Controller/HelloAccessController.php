<?php
/**
 * @file
 * Contains \Drupal\hello\Controller\HelloAccessController.
 */

namespace Drupal\hello\Controller;

use Drupal\Core\Controller\ControllerBase;

class HelloAccessController extends ControllerBase{

    public function content(){

        return array('#markup' => t('GG!! You are older than 6 years!!'));

    }
}