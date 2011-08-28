function v = electricParticleSpeed( q_1, q_2, epsilon_0, m_2, d, x)

% note as $x \rightarrow \infinity \Rightarrow 1/x \rightarrow 0$
v = ((q_1 * q_2)/(2 * pi * epsilon_0 * m_2)) * ( (1/d) - (1/x))
