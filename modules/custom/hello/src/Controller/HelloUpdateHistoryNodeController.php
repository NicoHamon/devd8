<?php
/**
 * @file
 * Contains \Drupal\hello\Controller\HelloListNodeController.
 */

namespace Drupal\hello\Controller;

use Drupal\Core\Controller\ControllerBase;
use Drupal\node\NodeInterface;


class HelloUpdateHistoryNodeController extends ControllerBase{
    public function getTitle(){
//        return 'plop';
    }
    public function content(NodeInterface $node)
    {
        $node_id=$node->id();
        $node_title = $node->getTitle();
        $database = \Drupal::database();
        $query = $database->select('hello_node_history','hnh')
            ->fields('hnh',array('update_time','uid'))
            ->condition('nid',$node_id,'=')
            ->execute();
        $results= $query->fetchAll();

      foreach ($results as $res){
          $storage = \Drupal::entityTypeManager()->getStorage('user');
          $user = $storage->load($res->uid);
          $user_id = $user->getAccountName();
          $time= date('D, d/m/Y - H:i',$res->update_time);

          $tab_res[]= array(
                'author'=> $user_id,
                'update_time' => $time
          );
      }

/*tab header*/
      $count = count($results);
/*tab of results*/
        $tab_update_history=array(
            '#theme' => 'table',
            '#class' => 'class_blabla',
 //           '#caption' => $node_title. '\'s update history :',
            '#header' => array('Update Author','Update Time'),
            '#rows' => $tab_res,
            '#cache' => array('max-age'=>'10')
        );

        $span_caption = array(
            '#theme' =>     'hello_history',
 //           '#data' =>      $tab_update_history,
            '#node_name' => $node_title,
            '#count'     => $count,
        );

        if($count== '0'){
            return array($span_caption);
        }
        else{
            return array($span_caption,$tab_update_history) ;
        }


    }
}