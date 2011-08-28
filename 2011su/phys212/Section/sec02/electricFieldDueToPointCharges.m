%  Four point charges $2q$, $q$, $q$, and $q$ are placed at the corners of a rectangle of dimensions $a$ and $3a$ as shown in the figure.  A fith charge $q$ is placd at the cneter of the rectangle.  Our tak is to compute the electric field at the center of the rectangle, and then determine the force on $Q$.  



% (b) 
% Without using your calculator, calculate $sin( theta)$, $cos( theta)$, and $tan( theta)$ where $theta$ is the angle definedin the diagram. (Express your answer algebraically.)

adjacent = a
opposite = a/2
hypotenous = sqrt ( adjacent^2 + opposite^2) 

sin( theta) = opposite/hypotenous
cos( theta) = adjacent/hypotenous
tan( theta) = opposite/adjacent

% (a)
% Each of the 4 charges at the corners contributes to the electtric fied $E$ at the cneter of the rectangle.  You'll have to add theses contributions by components.  First, start by finding the magnitudes of the electric field contributions you will need to add together.  

E_q = abs( k*q/(hypotenous)^2)

E_2q = abs(k*q/(hypotenous)^2)

% (c) 
% Now compute the x and y components of the relvant contributions to the electric field at the center of the rectangle.
E_qQ_x = E_q cos( theta)
E_2qQ_x = E_q cos( theta)

E_2qQ_y = E_q sin( theta)
E_2qQ_y = E_q sin( theta)

E_Q_x =(E_2qQ_x)+(E_qQ_x)+(-E_qQ_x)+(-E_qQ_x)
E_Q_y = (-E_2qQ_y)+(E_qQ_y)+(-E_qQ_y)+(E_qQ_y)

%(d) 
% What is the total electric field $E$ at the center of the rectangle, given the particular values $q = 3*mu*C$ and $a = 2$ cm?

E = abs( E_Q_x) + abs( E_Q_y)

% (e)
% Finally what is the force on the fifth charge $Q$ due to this electric Field, given $Q = 4*mu*C$ 


