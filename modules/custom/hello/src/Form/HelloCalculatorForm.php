<?php
/**
 * @file
 * Contains \Drupal\hello\Form\HelloCalculatorForm.
 */
namespace Drupal\hello\Form;

use Drupal\Core\Ajax\CssCommand;
use Drupal\Core\Ajax\HtmlCommand;
use Drupal\Core\Form\FormBase;
use Drupal\Core\Form\FormStateInterface;
use Drupal\Core\Ajax\AjaxResponse;

/**
 * Implements an calculator form
 */
class HelloCalculatorForm extends FormBase{

    /**
     * {@inheritdoc}
     */
    public function getFormID(){
        return 'Hello_calculator_form';
    }

    public function buildForm(array $form, FormStateInterface $form_state)
    {
        // TODO: Implement buildForm() method.
        $result = $form_state->getTemporaryValue('result');
        if(isset($result)) {

            $form['result'] = array(
                '#markup' => t('Result : ').$result,
            );

        }

        $form['first_value']= array(
            '#type'=>           'textfield',
            '#title'=>          t('First value : '),
            '#Description'=>    t('Enter the first value'),
            '#size'=>           '40',
            '#maxlengh'=>       '128',
            //'#default_value'=>  0,
            '#attributes'=>     array('placeholder' =>'First value'),
            '#ajax' =>          array(
                'callback'  =>  array($this, 'validateTextAjax'),
                'event' =>      'change',
            ),
            '#suffix'=> '<span class="text-message"></span>',

        );
        $form['operator']= array(
            '#type' =>          'radios',
            '#title' =>         t('Operator : '),
            '#default_value' => 0,
            '#options' =>       array(0 => t('Add'),1 => t('Substract'),2 => t('Multiply'), 3 => t('Divide')),
            '#ajax' =>          array(
                'callback'  =>  array($this, 'validateTextAjax'),
                'event' =>      'change',
            ),
        );
        $form['second_value']= array(
            '#type'=>           'textfield',
            '#title'=>          t('Second value'),
            '#Description'=>    t('Enter the second value : '),
            '#size'=>           '40',
            '#maxlengh'=>       '128',
            '#default_value'=>  0,
            '#ajax' =>          array(
                'callback'  =>  array($this, 'validateTextAjax'),
                'event' =>      'change',
            ),
            '#suffix'=> '<span class="text-message-2"></span>',
        );
        $form['checkbox_3']= array(
            '#type'=>           'checkbox',
            '#title'=>          t('Display third value'),
        );

        $form['third_value']= array(
            '#type'=>           'textfield',
            '#title'=>          t('Third value'),
            '#Description'=>    t('Enter the second value : '),
            '#size'=>           '40',
            '#maxlengh'=>       '128',
            '#default_value'=>  0,
            '#states'=>         array(
                'visible' =>   array(
                    'input#edit-checkbox-3'  =>  array('checked' => TRUE),
                ),
            ),
        );

        $form['select_mode'] = [
            '#type' => 'select',
            '#title' => t('Select the result \'s display mode :'),
            '#options' => [
                '1' => 'On this page',
                '2' => 'Rebuild',
                '3' => 'with redirection',
            ],
        ];

        $form['button_submit'] = array(
            '#type'=>           'submit',
            '#value'=>          t('Calculate'),
        );
        return $form;
    }

    public function validateTextAjax(array &$form, FormStateInterface $form_state)
    {
        $first_value = $form_state->getValue('first_value');
        $second_value = $form_state->getValue('second_value');
        $operator_value = $form_state->getValue('operator');
        $response = new AjaxResponse();
        $css_error = ['border' => '2px solid red'];
        $css_status = ['border' => '1px solid black'];

        if (!empty($first_value)and !is_numeric($first_value)) {

            $message = t('the first value has to be a numerical value');
            $response->addCommand(new CssCommand('#edit-first-value',$css_error));
            $response->addCommand(new HtmlCommand('.text-message',$message));
        }
        else{
            $message = t('');
            $response->addCommand(new CssCommand('#edit-first-value',$css_status));
            $response->addCommand(new HtmlCommand('.text-message',$message));
        }

        if (!empty($second_value) and !is_numeric($second_value) or (($operator_value == '3' && $second_value == '0'))) {
            if (!empty($second_value) and !is_numeric($second_value))
                $message = t('the second value has to be a numerical value');

            if ($operator_value == '3' && $second_value == '0')
                $message = t('the second value can\'t be equal to \'0\' if the operator is \'Divide\' ');

            $response->addCommand(new CssCommand('#edit-second-value',$css_error));
            $response->addCommand(new HtmlCommand('.text-message-2',$message));
        }else{
            $message = t('');
            $response->addCommand(new CssCommand('#edit-second-value',$css_status));
            $response->addCommand(new HtmlCommand('.text-message-2',$message));
        }

        return $response;
    }

    public function validateForm(array &$form, FormStateInterface $form_state)
    {
        parent::validateForm($form, $form_state); // TODO: Change the autogenerated stub

        $first_value = $form_state->getValue('first_value');
        $second_value = $form_state->getValue('second_value');
        $operator_value = $form_state->getValue('operator');

        if (!is_numeric($first_value))
            $form_state->setErrorByName('first_value', t('the first value has to be a numerical value'));
        if (!is_numeric($second_value))
            $form_state->setErrorByName('second_value', t('the second value has to be a numerical value'));
//kint($operator);
        if ($operator_value == '3' && $second_value == '0')
            $form_state->setErrorByName('second_value', t('the second value can\'t be equal to \'0\' if the operator is \'Divide\' '));
    }


    public function submitForm(array &$form, FormStateInterface $form_state)
    {
        // TODO: Implement submitForm() method.

        $first_value = $form_state->getValue('first_value');
        $second_value = $form_state->getValue('second_value');
        $operator_value = $form_state->getValue('operator');
        $select_mode = $form_state->getValue('select_mode');

        switch ($operator_value){
            case 0: $result = $first_value + $second_value;
                break;
            case 1: $result = $first_value - $second_value;
                break;
            case 2: $result= $first_value * $second_value;
                break;
            case 3: $result= $first_value / $second_value;
                break;
            default: $result = 'Error .... :\'( ';
        }

        switch ($select_mode){
            case 1: drupal_set_message(t('Result of your operation : ').$result, 'status');
                break;
            case 2: $form_state->setTemporaryValue('result',$result);
                    $form_state->setRebuild();
                break;
            case 3: $values = array('first_value' => $first_value,
                                    'second_value' => $second_value,
                                    'operator' => $operator_value,
                                    'result' => $result);
                    $form_state->setRedirect('hello.hello_result_calculator',$values);//array('data_operation'=>$values)
                break;
            default: $result = 'Error .... :\'( ';
        }

    }

}