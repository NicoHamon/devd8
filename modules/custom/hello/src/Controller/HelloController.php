<?php
/**
 * @file
 * Contains \Drupal\hello\Controller\HelloController.
 */

namespace Drupal\hello\Controller;

use Drupal\Core\Controller\ControllerBase;

class HelloController extends ControllerBase{

    public function content($param){

        $account = $this->currentUser();
        $nom= $account->getAccountName();
echo $param;
        if ($param!='no parameters') {
            return array('#markup' => t('Vous êtes sur la page Hello. Votre nom d\'utilisateur est @name, et voici le paramètre dans l\'url : @param . ', array('@name' => $nom,'@param' => $param)));
        }
        else{
            return array('#markup' => t('Vous êtes sur la page Hello. Votre nom d\'utilisateur est @name.', array('@name' => $nom)));
        }

    }
}