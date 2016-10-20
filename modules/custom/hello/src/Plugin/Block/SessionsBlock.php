<?php

/**
 * @file
 * Contains \Drupal\hello\Plugin\Block\SessionsBlock.
 */

namespace Drupal\hello\Plugin\Block;



use Drupal\Core\Block\BlockBase;
use Drupal\Core\Session\AccountInterface;
use Drupal\Core\Access\AccessResult;

/**
 * Provides a Sessions block.
 *
 * @Block(
 * id = "sessions_block",
 * admin_label = @Translation("Sessions actives")
 * )
 */


class SessionsBlock extends BlockBase{
    /**
     * Implements Drupal\Core\Block\BlockBase::build();
     */
    public function build(){
        $database = \Drupal::database();
        $sessions = $database->select('sessions','s')
            ->fields('s',array('sid'))
            ->countQuery()->execute();
        $session_num= $sessions->fetchField();
        return array('#markup' => t('Il y a actuellement @num session(s) active(s).', array('@num' => $session_num)),
            '#cache' => array('max-age'=>'10'));
    }

    protected function blockAccess(AccountInterface $account){
        if ($account->hasPermission('Access Hello')) {
            return AccessResult::allowed();
        }

        return AccessResult::forbidden();
    }

}