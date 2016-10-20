<?php
/**
 * @file
 * Contains \Drupal\hello\Controller\HelloRssController.
 */

namespace Drupal\hello\Controller;

use Drupal\Core\Controller\ControllerBase;
use Symfony\Component\HttpFoundation\Response;

class HelloRssController extends ControllerBase{

    public function content(){
        $response = new Response();
        $response->setContent('<xml>bla bla xml</xml>');
        return $response;


    }
}