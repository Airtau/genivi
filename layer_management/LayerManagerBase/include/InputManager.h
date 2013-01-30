/***************************************************************************
*
* Copyright 2012 Valeo
*
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*        http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
****************************************************************************/

#ifndef _INPUTMANAGER_H_
#define _INPUTMANAGER_H_


#include <map>
#include <vector>
#include <pthread.h>


class IScene;
class Surface;

#include "ilm_types.h"

/**
 * @brief Identifier for the different type of input devices supported by LayerManager.
 *        Fields can be used as a bit mask
 */
typedef ilmInputDevice InputDevice;
#define INPUT_DEVICE_KEYBOARD   ((InputDevice) ILM_INPUT_DEVICE_KEYBOARD)
#define INPUT_DEVICE_POINTER    ((InputDevice) ILM_INPUT_DEVICE_POINTER)
#define INPUT_DEVICE_TOUCH      ((InputDevice) ILM_INPUT_DEVICE_TOUCH)
#define INPUT_DEVICE_ALL        ((InputDevice) ILM_INPUT_DEVICE_ALL)


/**
 * @brief List the different states an input can be.
 */
typedef enum
{
    INPUT_STATE_PRESSED,    /*!< input is pressed    */
    INPUT_STATE_MOTION,     /*!< input is in motion  */
    INPUT_STATE_RELEASED,   /*!< input is released   */
    INPUT_STATE_OTHER       /*!< input is in an other, not listed, state  */
} InputEventState;


/**
 * @brief Type that describe a point.
 * Keep it POD (Plain Old Datatype) for performance reasons when dealing with touch event
 */
typedef struct
{
    InputEventState state;  /*<! State of the point       */
    int x;                  /*<! X position of the point  */
    int y;                  /*<! Y position of the point  */
}  Point;


/**
 * @brief Type to hold list of points
 */
typedef std::vector<Point> PointVect;
typedef PointVect::iterator PointVectIterator;


class InputManager
{
    public:
        /** Ctor / Dtor
         */
        InputManager(IScene* s);
        ~InputManager();


        /** Methods to report input events
         *  They all return the surface to transfer the event to, or NULL if the event should not be dispatched
         */
        Surface * reportKeyboardEvent(InputEventState state, long keyId);
        Surface * reportTouchEvent(PointVect& pv);
        Surface * reportPointerEvent(Point& p);


        /** Methods to control the focus
         */
        bool setKeyboardFocusOn(unsigned int surfId);
        bool updateInputEventAcceptanceOn(unsigned int surfId, InputDevice devices, bool accept);

        /** Few getters
         */
        unsigned int getKeyboardFocusSurfaceId();

    private:
        Surface * electSurfaceForPointerEvent(int& x, int& y);
        void transformGlobalToLocalCoordinates(Surface* surf, int& x, int& y);


        /*
         * Private Getters / Setters
         * Needed because access to their associated member requires exclusive area
         */

        /** \brief Set the keyboard focus on a particular surface */
        void _setKbdFocus(Surface * s);
        /** \brief Get the surface which has keyboard focus */
        Surface * _getKbdFocus();
        /** \brief Set the pointer focus on a particular surface */
        void _setPointerFocus(Surface * s);
        /** \brief Get the surface which has pointer focus */
        Surface * _getPointerFocus();
        /** \brief Set the touch focus on a particular surface */
        void _setTouchFocus(Surface * s);
        /** \brief Get the surface which has touch focus */
        Surface * _getTouchFocus();


    private:
        IScene * m_pScene;                  /*!< Pointer to the scene */
        std::map<long, Surface*> m_KeyMap;  /*!< Map that associate keypressed event to the surface it has been forward to. See @ref<InputManager-KeypressedMap>. */
        pthread_mutex_t m_mutex;            /*!< Mutex to avoid concurrent access to shared variables */

        /* Access to the below members must be protected by mutex to avoid concurrent accesses */
        Surface * m_KbdFocus;               /*!< Pointer to the surface which has the focus for keyboard event.
                                                 Should only be accessed via its getter / setter */
        Surface * m_PointerFocus;           /*!< Pointer to the surface which has the focus for pointer event.
                                                 Should only be accessed via its getter / setter */
        Surface * m_TouchFocus;             /*!< Pointer to the surface which has the focus for touch event.
                                                 Should only be accessed via its getter / setter */

};


/**
 * @section <InputManager-extra-documentation> (InputManager extra documentation)
 *
 * @subsection <InputManager-Requirements> (InputManager Requirements)
 *  <ul>
 *  \anchor<LM_INPUT_REQ_01>
 *    <li>
 *      LM_INPUT_REQ_01:
 *      LM should support input events dispatching to the relevant surface.
 *      Input devices can be keyboard, mouse or (multi)touch foil.
 *    </li>
 *
 *  \anchor<LM_INPUT_REQ_02>
 *    <li>
 *      LM_INPUT_REQ_02:
 *      Keyboard pressed events will be dispatched to the surface elected as being
 *      the "keyboard focused" surface.
 *      Keyboard released events will be dispatched to the surface which received
 *      the same key pressed event.
 *    </li>
 *
 *  \anchor<LM_INPUT_REQ_03>
 *    <li>
 *      LM_INPUT_REQ_03:
 *      A surface gain the Keyboard focus if the LM command "surfaceSetKeyboardFocus"
 *      is called on that surface. The elected surface can be changed at any time.
 *    </li>
 *
 *  \anchor<LM_INPUT_REQ_04>
 *  <li>
 *    LM_INPUT_REQ_04:
 *    Mouse & touch events will be dispatched to the surface elected as being the "Pointed" surface,
 *    even if theses events are outside of the surface. Coordinates will be adjusted relatively to the surface.
 *  </li>
 *
 *  \anchor<LM_INPUT_REQ_05>
 *  <li>
 *    LM_INPUT_REQ_05:
 *    A surface gain the "Pointed surface" status as soon as a Pointer event in "Pressed"
 *    state is reported under it.
 *    The conditions to gain this status is that the surface or its containing layer should:
 *     + be visible
 *     + be opaque (opacity != 0).
 *     + has a native content
 *  </li>
 *
 *  \anchor<LM_INPUT_REQ_06>
 *  <li>
 *    LM_INPUT_REQ_06:
 *    A surface can request to not receive particular input events. In this case, the surface should not be considered for focus election & the events
 *    must be dispatched to an other surface, if relevant.
 *  </li>
 *
 *  </ul>
 *
 *
 *
 *
 * @subsection <InputManager-KeypressedMap> (InputManager KeyPressed map)
 *
 * Note that reportKeyboardEvent() method takes a long as 2nd argument.
 * This is actually an identifier of the key being reported. It's purpose is to avoid a race in the following scenario :
 * 1- Surface S1 is the keyboard elected surface
 * 2- Key X is pressed, so we indicate the renderer to forward to S1
 * 3- The command "surfaceSetKeyboardFocus" is called on S2, so the surface S2 is now the keyboard elected one
 * 4- Key X is released.
 * We should then forward the KeyRelased event to S1 since we have reported the pressed event to it.
 * So we need a map that associate keyPressed  -> Surface. When the same key is released, we must forward that event to the original surface, and not to the elected one.
 *
 *
 */


#endif /* ! #ifndef _INPUTMANAGER_H_ */

