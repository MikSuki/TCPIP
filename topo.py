# sudo mn --custom topo.py --topo mytopo
# arp -d IP


from mininet.topo import Topo
 
class MyTopo( Topo ):
    "Simple topology example."
 
    def __init__( self ):
        "Create custom topo."
 
        # Initialize topology
        Topo.__init__( self )
 
        # Add hosts and switches
        h1 = self.addHost( 'h1' )
        h2 = self.addHost( 'h2' )
        h3 = self.addHost( 'h3' )
        h4 = self.addHost( 'h4' )
        h5 = self.addHost( 'h5' )
        h6 = self.addHost( 'h6' )
        h7 = self.addHost( 'h7' )
        h8 = self.addHost( 'h8' )
        h9 = self.addHost( 'h9' )
        h10 = self.addHost( 'h10' )
        s1 = self.addSwitch( 's1' )
 
        # Add links
        self.addLink( s1, h1 )
        self.addLink( s1, h3 )
        self.addLink( s1, h5 )
        self.addLink( s1, h7 )
        self.addLink( s1, h9 )
 
 
topos = { 'mytopo': ( lambda: MyTopo() ) }