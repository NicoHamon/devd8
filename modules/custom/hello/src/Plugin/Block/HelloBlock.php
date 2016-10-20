<?php

/**
 * @file
 * Contains \Drupal\hello\Plugin\Block\HelloBlock.
 */

namespace Drupal\hello\Plugin\Block;

use Drupal\Core\Block\BlockBase;

/**
 * Provides a hello block.
 *
 * @Block(
 * id = "hello_block",
 * admin_label = @Translation("Hello!")
 * )
 */
class HelloBlock extends BlockBase{
    /**
     * Implements Drupal\Core\Block\BlockBase::build();
     */
    public function build(){
        /*      $account = $this->currentUser();
                $nom= $account->getAccountName();*/
        $time=date('H\hi\ms\s');
        return array('#markup' => t('Bienvenue sur notre site. Il est @time.', array('@time' => $time)),
                    '#cache' => array('max-age'=>'10'));
    }
}