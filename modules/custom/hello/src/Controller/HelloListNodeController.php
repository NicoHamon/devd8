<?php
/**
 * @file
 * Contains \Drupal\hello\Controller\HelloListNodeController.
 */

namespace Drupal\hello\Controller;

use Drupal\Core\Controller\ControllerBase;


class HelloListNodeController extends ControllerBase{

    public function content($paramlist)
    {

        $storage = \Drupal::entityTypeManager()->getStorage('node');                        //charger le sYStm de stockage
        $query = \Drupal::entityQuery('node');

        if ($paramlist!='') {
            $query->condition('type', $paramlist, '=');
        }

        $ids=$query->pager('15')->execute();

        $entities = $storage->loadMultiple($ids);                                           //charger les objets à partir des ids

        $node_title=array();

        foreach ($entities as $entitie){
            /* @entitie NodeInterface $entitie */
            $node_title[]=$entitie->toLink();
            }

        $list = array(
            '#theme' => 'item_list',
            '#items' => $node_title,
            '#list_type' => 'ul',
            );
        $pager = array(
            '#type' => 'pager',
        );

        return array($list,$pager);


    }
}